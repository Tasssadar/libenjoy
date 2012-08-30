/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
**
**    See README and COPYING
***********************************************/

#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "libenjoy.h"
#include "libenjoy_linux.h"
#include "libenjoy_p.h"

struct libenjoy_os_ctx *libenjoy_init_private(void)
{
    libenjoy_os_ctx *os = (libenjoy_os_ctx*)malloc(sizeof(libenjoy_os_ctx));
    os->known_devs = (libenjoy_known_info **)calloc(1, sizeof(libenjoy_known_info*));
    os->invalid_ritr = 0;
    os->invalid_witr = 0;
    return os;
}

void libenjoy_close_private(struct libenjoy_os_ctx *os)
{
    if(os->known_devs)
    {
        libenjoy_known_info **i = os->known_devs;
        for(; *i != NULL; ++i)
        {
            free((*i)->path);
            free(*i);
        }
        free(os->known_devs);
    }
    free(os);
}

void libenjoy_enumerate(libenjoy_context *ctx)
{
    static const char* joydev_paths[] = {
        "/dev/input/js%u",
        "/dev/js%u"
    };

    char path[PATH_MAX];
    struct stat jstat;
    uint32_t j, i;
    char name[256] = { 0 };
    uint64_t hash;

    uint32_t *existing_ids = libenjoy_create_existing_ids(ctx);
    uint32_t existing_size = ctx->info_list.count;

    // Remove joysticks which returned ENODEV
    for(i = libenjoy_invalid_read_get(ctx->os); i != UINT_MAX; i = libenjoy_invalid_read_get(ctx->os))
    {
        libenjoy_destroy_joy_info(ctx, i);
        libenjoy_invalid_read_pop(ctx->os);
    }

    for(j = 0;j < LIBENJOY_MAX_JOYSTICK; ++j)
    {
        for(i = 0; i < sizeof(joydev_paths)/sizeof(joydev_paths[0]); ++i)
        {
            sprintf(path, joydev_paths[i], j);

            if(stat(path, &jstat) != 0)
                continue;

            int fd = open(path, O_RDONLY);
            if(fd == -1)
                continue;

            if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
                strncpy(name, "Unknown", sizeof(name));

            hash = libenjoy_hash(name);

            libenjoy_known_info *inf = libenjoy_get_known_hash(ctx->os, hash);
            if(inf == NULL)
                inf = libenjoy_add_known_id(ctx->os, hash, path);

            if(libenjoy_joy_info_created(ctx, inf->id) == 0)
            {
                // Update path if needed
                if(strcmp(inf->path, path) != 0)
                {
                    inf->path = (char*)realloc(inf->path, strlen(path)+1);
                    strcpy(inf->path, path);
                }

                libenjoy_joy_info *joy_inf = (libenjoy_joy_info*)malloc(sizeof(libenjoy_joy_info));
                joy_inf->id = inf->id;
                joy_inf->opened = 0;

                joy_inf->name = (char*)calloc((strlen(name)+1), sizeof(char));
                strcpy(joy_inf->name, name);

                libenjoy_add_joy_info(ctx, joy_inf);
            }
            // yeah, we are screwed. Two devices with same dev_id, just ignore the later one
            else if(libenjoy_set_id_exists(inf->id, existing_ids, existing_size) == 0)
            {
                // Update path if needed
                if(strcmp(inf->path, path) != 0)
                {
                    inf->path = (char*)realloc(inf->path, strlen(path)+1);
                    strcpy(inf->path, path);
                }
            }

            close(fd);
        }
    }

    // remove no longer existing joysticks
    for(i = 0; existing_ids && i < existing_size; ++i)
    {
        if(existing_ids[i] == UINT_MAX)
            continue;
        libenjoy_destroy_joy_info(ctx, existing_ids[i]);
    }
    free(existing_ids);
}

libenjoy_known_info *libenjoy_get_known_hash(libenjoy_os_ctx *octx, uint64_t hash)
{
    libenjoy_known_info **i = octx->known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->name_hash == hash)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_get_known_id(libenjoy_os_ctx *octx, uint32_t id)
{
    libenjoy_known_info **i = octx->known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->id == id)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_add_known_id(libenjoy_os_ctx *octx, uint64_t hash, char *path)
{
    uint32_t count = 2;
    libenjoy_known_info **i = octx->known_devs;
    for(; *i != NULL; ++i)
        ++count;

    octx->known_devs = (libenjoy_known_info**)realloc(octx->known_devs, sizeof(libenjoy_known_info*)*count);
    count -= 2;

    libenjoy_known_info *inf = (libenjoy_known_info*)malloc(sizeof(libenjoy_known_info));
    inf->name_hash = hash;
    inf->id = libenjoy_get_new_joyid();
    inf->path = (char*)malloc(strlen(path)+1);
    strcpy(inf->path, path);

    octx->known_devs[count++] = inf;
    octx->known_devs[count] = NULL;
    return inf;
}

uint32_t *libenjoy_create_existing_ids(libenjoy_context *ctx)
{
    uint32_t *res = NULL;
    uint32_t i = 0;

    if(!ctx->info_list.list)
        return res;

    res = (uint32_t*)malloc(ctx->info_list.count*sizeof(uint32_t));
    for(; i < ctx->info_list.count; ++i)
        res[i] = ctx->info_list.list[i]->id;
    return res;
}

int libenjoy_set_id_exists(uint32_t id, uint32_t *list, uint32_t size)
{
    uint32_t i = 0;

    for(; list && i < size; ++i)
    {
        if(list[i] == id)
        {
            list[i] = UINT_MAX;
            return 0;
        }
    }
    return -1;
}

libenjoy_os_specific *libenjoy_open_os_specific(libenjoy_context *ctx, uint32_t id)
{
    libenjoy_known_info *inf = libenjoy_get_known_id(ctx->os, id);
    if(!inf)
        return NULL;

    int fd = open(inf->path, (O_RDONLY | O_NONBLOCK));
    if(fd == -1)
        return NULL;

    // HACK FIX: read correction values and then set them.
    // this will cause joydev driver to read axes values, so that
    // JS_EVENT_INIT is correct.
    char axes;
    if(ioctl(fd, JSIOCGAXES, &axes) < 0)
        axes = 64;

    struct js_corr *corr = (struct js_corr*)calloc(axes, sizeof(struct js_corr));
    if(ioctl(fd, JSIOCGCORR, corr) >= 0)
        ioctl(fd, JSIOCSCORR, corr);
    free(corr);

    libenjoy_os_specific *res = (libenjoy_os_specific*)malloc(sizeof(libenjoy_os_specific));
    res->fd = fd;
    return res;
}

void libenjoy_close_os_specific(libenjoy_os_specific *os)
{
    close(os->fd);
    free(os);
}

void libenjoy_set_parts_count(libenjoy_joystick *joy)
{
    char val = 0;

    ioctl(joy->os->fd, JSIOCGBUTTONS, &val);
    joy->num_buttons = val;

    ioctl(joy->os->fd, JSIOCGAXES, &val);
    joy->num_axes = val;
}

void libenjoy_poll_priv(libenjoy_context *ctx)
{
    struct js_event e;
    libenjoy_joystick *joy;
    uint32_t i = 0;

    for(; i < ctx->joy_list.count; ++i)
    {
        joy = ctx->joy_list.list[i];
        if(joy->valid == 0)
            continue;

        while (read (joy->os->fd, &e, sizeof(struct js_event)) > 0)
        {
            // data in INIT event are WRONG. Mostly. Joystick driver bug?
            // hackfixed - see libenjoy_open_os_specific()
            //if(e.type & JS_EVENT_INIT)
            //    continue;

            libenjoy_event *ev = libenjoy_buff_get_for_write(ctx);
            ev->joy_id = joy->id;

            if(e.type & JS_EVENT_AXIS)
                ev->type = LIBENJOY_EV_AXIS;
            else if(e.type & JS_EVENT_BUTTON)
                ev->type = LIBENJOY_EV_BUTTON;
            else
                continue;

            ev->part_id = e.number;
            ev->data = e.value;

            libenjoy_buff_push(ctx);
        }

        if(errno == ENODEV)
        {
            libenjoy_invalidate_joystick(joy);
            libenjoy_invalid_read_add(ctx->os, joy->id);
        }
    }
}

void libenjoy_invalid_read_add(libenjoy_os_ctx *octx, uint32_t id)
{
    uint16_t new_itr = libenjoy_invalid_inc_if_can(octx->invalid_witr);

    if(new_itr == octx->invalid_ritr)
        libenjoy_invalid_read_pop(octx);

    octx->invalid_reads[octx->invalid_witr] = id;

    octx->invalid_witr = new_itr;
}

uint32_t libenjoy_invalid_read_get(libenjoy_os_ctx *octx)
{
    if(octx->invalid_witr == octx->invalid_ritr)
        return UINT_MAX;

    return octx->invalid_reads[octx->invalid_ritr];
}

void libenjoy_invalid_read_pop(libenjoy_os_ctx *octx)
{
    if(octx->invalid_witr != octx->invalid_ritr)
        octx->invalid_ritr = libenjoy_invalid_inc_if_can(octx->invalid_ritr);
}

uint8_t libenjoy_invalid_inc_if_can(uint8_t val)
{
    ++val;
    return val == LIBENJOY_MAX_JOYSTICK ? 0 : val;
}

uint64_t libenjoy_hash(char *str)
{
    uint64_t h = 0;
    for(; *str; ++str)
    {
        if(*str != ' ')
            h = h << 1 ^ *str;
    }
    return h;
}
