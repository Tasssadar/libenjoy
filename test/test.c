#ifdef __linux
    #include <unistd.h>
#else
    #include <windows.h>
#endif
#include <stdio.h>

#pragma comment(lib, "winmm.lib")

#include "../src/libenjoy.h"

int main()
{
    struct libenjoy_joystick *j;
    libenjoy_event ev;
    uint32_t counter = 0;
    uint32_t counter2 = 0;
    
    libenjoy_init();
    libenjoy_enumerate();
    
    printf("enumerating..\n");
    j = libenjoy_open_joystick(0);

    if(!j)
    {
        libenjoy_close();
        return 0;
    }

    while(1)
    {
        /*
        libenjoy_joy_info_list *info;
        uint32_t i;

        libenjoy_enumerate();
        info = libenjoy_get_info_list();

        if(j)
        {
            printf("    Valid: %d\n", j->valid);
            printf("    axes: %d btns: %d\n", libenjoy_get_axes_num(j), libenjoy_get_buttons_num(j));
        }

        for(i = 0; i < info->count; ++i)
        {
            printf("%u: %s\n", info->list[i]->id, info->list[i]->name);
        }
        libenjoy_free_info_list(info);

#ifdef __linux
        sleep(1);
#else
        Sleep(1000);
#endif
        */
        
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
#ifdef __linux
        usleep(50000);
#else
        Sleep(50);
#endif
        counter += 50;
        counter2 += 50;

        if(counter >= 1000)
        {
            libenjoy_enumerate();
            counter = 0;
        }

        if(counter2 >= 10000)
            break;
    }
    libenjoy_close_joystick(j);
    libenjoy_close();
    return 0;
}
