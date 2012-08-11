#include <unistd.h>
#include <stdio.h>
#include "../src/libenjoy.h"

int main()
{
    printf("enumerating..\n");
    while(1)
    {
        libenjoy_enumerate();
        libenjoy_joy_info_list *info = libenjoy_get_info_list();
        uint32_t i = 0;
        for(; i < info->count; ++i)
        {
            printf("%u: %s\n", info->list[i]->id, info->list[i]->name);
        }
        libenjoy_free_info_list(info);
        printf("\n");
        sleep(1);
    }
    return 0;
}
