/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
**
**    See README and COPYING
***********************************************/

#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct libenjoy_os_specific libenjoy_os_specific;

#include "libenjoy.h"
#include "libenjoy_p.h"

struct libenjoy_context *libenjoy_init(void)
{
    libenjoy_context *ctx = (libenjoy_context*)malloc(sizeof(libenjoy_context));
    ctx->info_list.count = 0;
    ctx->info_list.list = NULL;

    ctx->joy_list.count = 0;
    ctx->joy_list.list = NULL;

    ctx->buff_wr_itr = 0;
    ctx->buff_rd_itr = 0;

    ctx->os = libenjoy_init_private();
    return ctx;
}

void libenjoy_close(libenjoy_context *ctx)
{
    uint32_t i;

    if(!ctx)
        return;

    libenjoy_close_private(ctx->os);

    if(ctx->info_list.list != NULL)
    {
        for(i = 0; i < ctx->info_list.count; ++i)
        {
            free(ctx->info_list.list[i]->name);
            free(ctx->info_list.list[i]);
        }
        free(ctx->info_list.list);
    }

    if(ctx->joy_list.list != NULL)
    {
        while(ctx->joy_list.count != 0)
            libenjoy_close_joystick(ctx->joy_list.list[0]);
        free(ctx->joy_list.list);
    }
    free(ctx);
}

uint32_t libenjoy_get_new_joyid(void)
{
    static uint32_t counter = 0;
    return counter++;
}

int libenjoy_joy_info_created(libenjoy_context *ctx, uint32_t id)
{
    uint32_t i = 0;
    for(; i < ctx->info_list.count; ++i)
        if(id == ctx->info_list.list[i]->id)
            return 1;
    return 0;
}

libenjoy_joy_info* libenjoy_get_joy_info(libenjoy_context *ctx, uint32_t id)
{
    uint32_t i = 0;
    libenjoy_joy_info_list *list = &ctx->info_list;

    for(; i < list->count; ++i)
        if(id == list->list[i]->id)
            return list->list[i];
    return NULL;
}

void libenjoy_add_joy_info(libenjoy_context *ctx, libenjoy_joy_info *inf)
{
    libenjoy_joystick *joy = libenjoy_get_joystick(ctx, inf->id);
    libenjoy_joy_info_list *list = &ctx->info_list;

    list->list = (libenjoy_joy_info**)realloc(list->list, (list->count+1)*sizeof(libenjoy_joy_info*));
    list->list[list->count++] = inf;

    // try to reopen old joystick
    if(joy && joy->valid == 0)
    {
        joy->os = libenjoy_open_os_specific(ctx, inf->id);
        if(joy->os)
        {
            // sent disconnect event
            libenjoy_event *ev = libenjoy_buff_get_for_write(ctx);
            ev->joy_id = inf->id;
            ev->type = LIBENJOY_EV_CONNECTED;
            ev->data = 1;
            libenjoy_buff_push(ctx);

            inf->opened = 1;
            joy->valid = 1;
        }
    }
}

void libenjoy_destroy_joy_info(libenjoy_context *ctx, uint32_t id)
{
    uint32_t i = 0;
    libenjoy_joy_info_list *list = &ctx->info_list;
    for(; i < list->count; ++i)
    {
        if(list->list[i]->id != id)
            continue;

        libenjoy_joy_set_valid_by_id(ctx, id, 0);
        free(list->list[i]->name);
        free(list->list[i]);

        --list->count;
        if(list->count == 0)
        {
            free(list->list);
            list->list = NULL;
            return;
        }

        if(i != list->count)
            list->list[i] = list->list[list->count];

        list->list = (libenjoy_joy_info**)realloc(list->list, list->count*sizeof(libenjoy_joy_info*));
        return;
    }
}

libenjoy_joy_info_list *libenjoy_get_info_list(libenjoy_context *ctx)
{
    uint32_t i;
    libenjoy_joy_info_list *res = (libenjoy_joy_info_list*)malloc(sizeof(libenjoy_joy_info_list));
    libenjoy_joy_info_list *list = &ctx->info_list;

    res->count = list->count;

    if(list->count == 0)
    {
        res->list = NULL;
        return res;
    }
    
    res->list = (libenjoy_joy_info**)calloc(res->count, sizeof(libenjoy_joy_info*));
    for(i = 0; i < list->count; ++i)
    {
        res->list[i] = (libenjoy_joy_info*)malloc(sizeof(libenjoy_joy_info));
        res->list[i]->id = list->list[i]->id;
        res->list[i]->name = (char*)malloc(strlen(list->list[i]->name)+1);
        strcpy(res->list[i]->name, list->list[i]->name);
    }
    return res;
}

void libenjoy_free_info_list(libenjoy_joy_info_list *list)
{
    uint32_t i = 0;
    for(; i < list->count; ++i)
    {
        free(list->list[i]->name);
        free(list->list[i]);
    }
    free(list->list);
    free(list);
}

libenjoy_joystick *libenjoy_open_joystick(libenjoy_context *ctx, uint32_t id)
{
    libenjoy_joy_info* info = libenjoy_get_joy_info(ctx, id);
    libenjoy_os_specific *os;
    libenjoy_joystick *joy;

    if(!info || info->opened != 0)
        return NULL;

    os = libenjoy_open_os_specific(ctx, id);
    if(!os)
        return NULL;

    joy = (libenjoy_joystick*)malloc(sizeof(libenjoy_joystick));
    joy->id = id;
    joy->os = os;
    joy->valid = 1;
    joy->ctx = ctx;
    libenjoy_add_joy_to_list(joy);

    info->opened = 1;
    return joy;
}

void libenjoy_close_joystick(libenjoy_joystick *joy)
{
    if(joy->os)
        libenjoy_close_os_specific(joy->os);

    if(joy->valid == 1)
    {
        libenjoy_joy_info* info = libenjoy_get_joy_info(joy->ctx, joy->id);
        if(info)
            info->opened = 0;
    }

    libenjoy_rm_joy_from_list(joy);
    free(joy);
}

void libenjoy_add_joy_to_list(libenjoy_joystick *joy)
{
    libenjoy_joystick_list *jlist = &joy->ctx->joy_list;
    jlist->list = (libenjoy_joystick**)realloc(jlist->list, ++jlist->count*sizeof(libenjoy_joystick*));
    jlist->list[jlist->count-1] = joy;
}

void libenjoy_rm_joy_from_list(libenjoy_joystick *joy)
{
    uint32_t i = 0;
    libenjoy_joystick_list *jlist = &joy->ctx->joy_list;
    for(;i < jlist->count; ++i)
    {
        if(jlist->list[i] != joy)
            continue;

        --jlist->count;

        if(i != jlist->count)
            jlist->list[i] = jlist->list[jlist->count];

        jlist->list = (libenjoy_joystick**)realloc(jlist->list, jlist->count*sizeof(libenjoy_joystick*));
        return;
    }
}

void libenjoy_joy_set_valid_by_id(libenjoy_context *ctx, uint32_t id, char valid)
{
    libenjoy_joystick *joy = libenjoy_get_joystick(ctx, id);
    if(joy)
        libenjoy_joy_set_valid(joy, valid);
}

void libenjoy_joy_set_valid(libenjoy_joystick *joy, char valid)
{
    if(joy->valid == 1 && valid == 0)
    {
        libenjoy_event *ev = libenjoy_buff_get_for_write(joy->ctx);

        joy->valid = 0;
        libenjoy_close_os_specific(joy->os);
        joy->os = NULL;

        // sent disconnect event
        ev->joy_id = joy->id;
        ev->type = LIBENJOY_EV_CONNECTED;
        ev->data = 0;
        libenjoy_buff_push(joy->ctx);
    }
    else
        joy->valid = valid;
}

libenjoy_joystick *libenjoy_get_joystick(libenjoy_context *ctx, uint32_t id)
{
    uint32_t i = 0;
    for(;i < ctx->joy_list.count; ++i)
    {
        if(ctx->joy_list.list[i]->id == id)
            return ctx->joy_list.list[i];
    }
    return NULL;
}

void libenjoy_buff_push(libenjoy_context *ctx)
{
    uint16_t new_itr = libenjoy_buff_inc_if_can(ctx->buff_wr_itr);

    if(new_itr == ctx->buff_rd_itr)
        libenjoy_buff_pop(ctx);

    ctx->buff_wr_itr = new_itr;
}

libenjoy_event *libenjoy_buff_get_for_write(libenjoy_context *ctx)
{
    return &ctx->event_buffer[ctx->buff_wr_itr];
}

libenjoy_event libenjoy_buff_top(libenjoy_context *ctx)
{
    return ctx->event_buffer[ctx->buff_rd_itr];
}

void libenjoy_buff_pop(libenjoy_context *ctx)
{
   ctx->buff_rd_itr = libenjoy_buff_inc_if_can(ctx->buff_rd_itr);
}

uint16_t libenjoy_buff_inc_if_can(uint16_t val)
{
    ++val;
    return (val == EVENT_BUFFER_SIZE) ? 0 : val;
}

int libenjoy_poll(libenjoy_context *ctx, libenjoy_event *ev)
{
    libenjoy_poll_priv(ctx);

    if(ctx->buff_rd_itr == ctx->buff_wr_itr)
        return 0;

    *ev = libenjoy_buff_top(ctx);
    libenjoy_buff_pop(ctx);
    return 1;
}

