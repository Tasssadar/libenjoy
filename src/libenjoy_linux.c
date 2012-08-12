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

static libenjoy_known_info **known_devs = NULL;

void libenjoy_init_private(void)
{
    known_devs = (libenjoy_known_info **)calloc(1, sizeof(libenjoy_known_info*));
}

void libenjoy_close_private(void)
{
    if(known_devs)
    {
        libenjoy_known_info **i = known_devs;
        for(; *i != NULL; ++i)
        {
            free((*i)->path);
            free(*i);
        }
        free(known_devs);
        known_devs = NULL;
    }
}

void libenjoy_enumerate(void)
{
    static const char* joydev_paths[] = {
        "/dev/input/js%u",
        "/dev/js%u"
    };

    char path[PATH_MAX];
    struct stat jstat;
    uint32_t j, i;

    uint32_t *existing_ids = libenjoy_create_existing_ids();
    uint32_t existing_size = joy_info.count;

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

            libenjoy_known_info *inf = libenjoy_get_known_devid(jstat.st_rdev);
            if(inf == NULL)
                inf = libenjoy_add_known_id(jstat.st_rdev, path);

            if(libenjoy_joy_info_created(inf->id) != 0)
            {
                libenjoy_joy_info *joy_inf = (libenjoy_joy_info*)malloc(sizeof(libenjoy_joy_info));
                joy_inf->id = inf->id;
                joy_inf->opened = 0;

                char name[128] = { 0 };
                if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
                    strncpy(name, "Unknown", sizeof(name));

                joy_inf->name = (char*)calloc((strlen(name)+1), sizeof(char));
                strcpy(joy_inf->name, name);

                libenjoy_add_joy_info(joy_inf);
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
        libenjoy_destroy_joy_info(existing_ids[i]);
    }
    free(existing_ids);
}

libenjoy_known_info *libenjoy_get_known_devid(dev_t devid)
{
    libenjoy_known_info **i = known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->devid == devid)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_get_known_id(uint32_t id)
{
    libenjoy_known_info **i = known_devs;
    for(; *i != NULL; ++i)
    {
        if((*i)->id == id)
            return *i;
    }
    return NULL;
}

libenjoy_known_info *libenjoy_add_known_id(dev_t devid, char *path)
{
    uint32_t count = 2;
    libenjoy_known_info **i = known_devs;
    for(; *i != NULL; ++i)
        ++count;

    known_devs = (libenjoy_known_info**)realloc(known_devs, sizeof(libenjoy_known_info*)*count);
    count -= 2;

    libenjoy_known_info *inf = (libenjoy_known_info*)malloc(sizeof(libenjoy_known_info));
    inf->devid = devid;
    inf->id = libenjoy_get_new_joyid();
    inf->path = (char*)malloc(strlen(path)+1);
    strcpy(inf->path, path);

    known_devs[count++] = inf;
    known_devs[count] = NULL;
    return inf;
}

uint32_t *libenjoy_create_existing_ids()
{
    uint32_t *res = NULL;
    uint32_t i = 0;

    if(!joy_info.list)
        return res;

    res = (uint32_t*)malloc(joy_info.count*sizeof(uint32_t));
    for(; i < joy_info.count; ++i)
        res[i] = joy_info.list[i]->id;
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

libenjoy_os_specific *libenjoy_open_os_specific(uint32_t id)
{
    libenjoy_known_info *inf = libenjoy_get_known_id(id);
    if(!inf)
        return NULL;

    int fd = open(inf->path, (O_RDONLY | O_NONBLOCK));
    if(fd == -1)
        return NULL;

    libenjoy_os_specific *res = (libenjoy_os_specific*)malloc(sizeof(libenjoy_os_specific));
    res->fd = fd;
    return res;
}

void libenjoy_close_os_specific(libenjoy_os_specific *os)
{
    close(os->fd);
    free(os);
}

int libenjoy_get_axes_num(libenjoy_joystick *joy)
{
    char num = 0;
    if(joy->os)
        ioctl(joy->os->fd, JSIOCGAXES, &num);
    return num;
}

int libenjoy_get_buttons_num(libenjoy_joystick *joy)
{
    char num = 0;
    if(joy->os)
        ioctl(joy->os->fd, JSIOCGBUTTONS, &num);
    return num;
}

void libenjoy_poll_priv(void)
{
    struct js_event e;
    libenjoy_joystick *joy;
    uint32_t i = 0;

    for(; i < joy_list.count; ++i)
    {
        joy = joy_list.list[i];
        if(joy->valid == 0)
            continue;

        while (read (joy->os->fd, &e, sizeof(struct js_event)) > 0)
        {
            if(e.type & JS_EVENT_INIT) // FIXME: correct?
                continue;

            libenjoy_event *ev = libenjoy_buff_get_for_write();
            ev->joy_id = joy->id;

            if(e.type & JS_EVENT_AXIS)
                ev->type = LIBENJOY_EV_AXIS;
            else if(e.type & JS_EVENT_BUTTON)
                ev->type = LIBENJOY_EV_BUTTON;
            else
                continue;

            ev->part_id = e.number;
            ev->data = e.value;

            libenjoy_buff_push();
        }
    }
}
