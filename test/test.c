#include <stdio.h>
#ifdef __linux
  #include <unistd.h>
#else
  #include <windows.h>
#endif

#include "../src/libenjoy.h"

// This tels msvc to link agains winmm.lib. Pretty nasty though.
#pragma comment(lib, "winmm.lib")

int main()
{
    libenjoy_context *ctx = libenjoy_init(); // initialize the library
    libenjoy_joy_info_list *info;

    // Updates internal list of joysticks. If you want auto-reconnect
    // after re-plugging the joystick, you should call this every 1s or so
    libenjoy_enumerate(ctx);

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
    info = libenjoy_get_info_list(ctx);

    if(info->count != 0) // just get the first joystick
    {
        libenjoy_joystick *joy;
        printf("Opening joystick %s...", info->list[0]->name);
        joy = libenjoy_open_joystick(ctx, info->list[0]->id);
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
                while(libenjoy_poll(ctx, &ev))
                {
                    switch(ev.type)
                    {
                    case LIBENJOY_EV_AXIS:
                        printf("%u: axis %d val %d\n", ev.joy_id, ev.part_id, ev.data);
                        break;
                    case LIBENJOY_EV_BUTTON:
                        printf("%u: button %d val %d\n", ev.joy_id, ev.part_id, ev.data);
                        break;
                    case LIBENJOY_EV_CONNECTED:
                        printf("%u: status changed: %d\n", ev.joy_id, ev.data);
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
                    libenjoy_enumerate(ctx);
                    counter = 0;
                }
            }

            // Joystick is really closed in libenjoy_poll or libenjoy_close,
            // because closing it while libenjoy_poll is in process in another thread
            // could cause crash. Be sure to call libenjoy_poll(ctx, NULL); (yes,
            // you can use NULL as event) if you will not poll nor libenjoy_close
            // anytime soon.
            libenjoy_close_joystick(joy);
        }
        else
            printf("Failed!\n");
    }
    else
        printf("No joystick available\n");

    // Frees memory allocated by that joystick list. Do not forget it!
    libenjoy_free_info_list(info);

    // deallocates all memory used by lib. Do not forget this!
    // libenjoy_poll must not be called during or after this call
    libenjoy_close(ctx);
    return 0;
}
