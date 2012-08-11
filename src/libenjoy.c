#include <stdlib.h>
#include <string.h>

#include "libenjoy.h"
#include "libenjoy_p.h"

struct libenjoy_joy_info_list joy_info;

void libenjoy_init(void)
{
    joy_info.count = 0;
    joy_info.list = NULL;

    libenjoy_init_private();
}

uint32_t libenjoy_get_new_joyid(void)
{
    static uint32_t counter = 0;
    return counter++;
}

int libenjoy_joy_info_created(uint32_t id)
{
    uint32_t i = 0;
    for(; i < joy_info.count; ++i)
        if(id == joy_info.list[i]->id)
            return 0;
    return -1;
}

void libenjoy_add_joy_info(libenjoy_joy_info *inf)
{
    joy_info.list = (libenjoy_joy_info**)realloc(joy_info.list, ++joy_info.count*sizeof(libenjoy_joy_info*));
    joy_info.list[joy_info.count-1] = inf;
}

void libenjoy_destroy_joy_info(uint32_t id)
{
    uint32_t i = 0;
    for(; i < joy_info.count; ++i)
    {
        if(joy_info.list[i]->id != id)
            continue;

        free(joy_info.list[i]);
        --joy_info.count;
        if(joy_info.count == 0)
        {
            free(joy_info.list);
            joy_info.list = NULL;
            return;
        }

        if(i != joy_info.count)
            joy_info.list[i] = joy_info.list[joy_info.count];

        joy_info.list = (libenjoy_joy_info**)realloc(joy_info.list, joy_info.count*sizeof(libenjoy_joy_info*));
        return;
    }
}

libenjoy_joy_info_list *libenjoy_get_info_list(void)
{
    libenjoy_joy_info_list *res = (libenjoy_joy_info_list*)malloc(sizeof(libenjoy_joy_info_list));
    res->count = joy_info.count;
    res->list = (libenjoy_joy_info**)calloc(res->count, sizeof(libenjoy_joy_info*));

    uint32_t i = 0;
    for(; i < joy_info.count; ++i)
    {
        res->list[i] = (libenjoy_joy_info*)malloc(sizeof(libenjoy_joy_info));
        res->list[i]->id = joy_info.list[i]->id;
        res->list[i]->name = (char*)malloc(strlen(joy_info.list[i]->name)+1);
        strcpy(res->list[i]->name, joy_info.list[i]->name);
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

