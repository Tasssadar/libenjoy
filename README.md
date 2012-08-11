# libenjoy
Lighweight (I mean simple) joystick library for Linux and Windows. It is desinged
for use in [Lorris](https://github.com/Tasssadar/Lorris), so it is
the easiest to use there - just  ´include(libenjoy.pri)´ in your .pro file.
It can be however used with any C/C++ app, just ´#include "libenjoy.h´ and make
sure that libenjoy.c and libenjoy_linux.c/libenjoy_win32.c are compiled.

Oh, yeah, and on Windows, you have to link with winmm.lib!

### Usage:
```C
#include <stdio.h>
#ifdef __linux
  #include <unistd.h>
#else
  #include <windows.h>
#endif

#include "libenjoy.h"

int main()
{
    libenjoy_init(); // initializes the library

    // Updates internal list of joysticks. If you want auto-reconnect
    // after re-plugging the joystick, you should call this every 1s or so
    libenjoy_enumerate();

    {
        // get list with available joysticks. structs are
        // typedef struct libenjoy_joy_info_list {
        //     uint32_t count;
        //     libenjoy_joy_info **list;
        // } libenjoy_joy_info_list;
        //
        // typedef struct libenjoy_joy_info {
        //     char *name;
        //     uint32_t id;
        //     char opened;
        // } libenjoy_joy_info;
        //
        // id is not linear (eg. you should not use vector or array), 
        // and if you disconnect joystick and then plug it in again,
        // it should have the same ID
        libenjoy_joy_info_list *info = libenjoy_get_info_list();

        if(info->count != 0) // just get the first joystick
        {
            libenjoy_joystick *joy;
            printf("Opening joystick %s...", info->list[0]->name);
            joy = libenjoy_open_joystick(info->list[0]->id);
            if(joy)
            {
                int counter = 0;
                libenjoy_event ev;

                printf("Success!\n");
                printf("Axes: %d btns: %d\n", libenjoy_get_axes_num(joy),libenjoy_get_buttons_num(joy));

                while(1)
                {
                    // Value data are not stored in library. if you want to use
                    // them, you have to store them

                    // That's right, only polling available
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
                    if(counter >= 1000)
                    {
                        libenjoy_enumerate();
                        counter = 0;
                    }
                }
                
                libenjoy_close_joystick(joy);
            }
            else
                printf("Failed!\n");

            // Frees memory allocated by that joystick list. Do not forget it!
            libenjoy_free_info_list(info);
        }
        else
            printf("No joystick available\n");
    }

    libenjoy_close(); // deallocates all memory used by lib. Do not forget this!
    return 0;
}
```

### License
LGPLv2.1, see COPYING. Most of libenjoy_win32.c is from libSDL - thanks!
