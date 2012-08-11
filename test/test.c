#include <unistd.h>
#include <stdio.h>
#include "../src/libenjoy.h"

int main()
{
    libenjoy_init();
    libenjoy_enumerate();
    printf("enumerating..\n");
    libenjoy_joystick *j = libenjoy_open_joystick(0);

    if(!j)
        return 0;

    libenjoy_event ev;
    uint32_t counter = 0;
    while(1)
    {

        /*libenjoy_enumerate();
        libenjoy_joy_info_list *info = libenjoy_get_info_list();

        if(j)
        {
            printf("    Valid: %d\n", j->valid);
            printf("    axes: %d btns: %d\n", libenjoy_get_axes_num(j), libenjoy_get_buttons_num(j));
        }

        uint32_t i = 0;
        for(; i < info->count; ++i)
        {
            printf("%u: %s\n", info->list[i]->id, info->list[i]->name);

        }
        libenjoy_free_info_list(info);
        sleep(1);*/


        while(libenjoy_poll(&ev) == 0)
        {
            switch(ev.type)
            {
                case LIBENJOY_EV_AXIS:
                    printf("%u: axis %d val %d\n", ev.joy_id, ev.part_id, ev.data);
                    break;
                case LIBENJOY_EV_BUTTON:
                    printf("%u: button %d val %d\n", ev.joy_id, ev.part_id, ev.data);
                    break;
            }
        }
        usleep(50000);
        counter += 50;

        if(counter >= 1000)
        {
            libenjoy_enumerate();
            counter = 0;
        }
    }
    libenjoy_close_joystick(j);
    libenjoy_close();
    return 0;
}
