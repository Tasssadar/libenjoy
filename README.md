# libenjoy
[![Build Status](https://travis-ci.org/Tasssadar/libenjoy.png?branch=master)](https://travis-ci.org/Tasssadar/libenjoy)

Lighweight (I mean simple) joystick library for Linux and Windows. It is desinged
for use in [Lorris](https://github.com/Tasssadar/Lorris), so it is
the easiest to use with Qt - just  `include(libenjoy.pri)` in your .pro file.
It can be however used with any C/C++ app, just `#include "libenjoy.h"` and make
sure that `libenjoy.c` and `libenjoy_linux.c`/`libenjoy_win32.c` are compiled.

Oh, yeah, and on Windows, you have to link with winmm.lib!

####Highlights
* Small. All files combined (both Linux and Windows) have about 1000 lines.
* Almost no additional dependencies.
  * On Linux, it is nothing other than GCC and kernel 2.2+
  * winmm.lib on Windows - nothing special
* Remembers joysticks. Joystick ID is unique, and libenjoy can automatically
  reconnect re-plugged joysticks. This works flawlessly on Linux, Windows
  on the other hand does not like it very much. Be sure to re-plug joysticks
  to the same USB port when using multiple joysticks at once.
* __Thread-safety note__: all functions (for one context) should be called
  from the same thread. The only exception is `libenjoy_poll`, which is made
  thread safe, so that you can make another thread to poll joystick events.

###WARNING!
The fact that **libenjoy** can handle re-plugged joystick means it
**cannot handle two or more exactly same joysticks at once**. It will pick
the one which was plugged-in first and ignore the other ones. This is because
without libusb, I can't get really unique device id, so there is no way for me
to identify more than one joystick of same type. But the situation "I just
wanna to fix the joystick's cable without restarting the app" is more frequent
than "Hey, let's buy twelve exactly same joysticks!", at least for me - that is
why I went on with this solution.

### Usage:
```C
#include <stdio.h>
#ifdef __linux
  #include <unistd.h>
#else
  #include <windows.h>
#endif

#include "libenjoy.h"

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
```

### License
LGPLv2.1, see COPYING. Most of libenjoy_win32.c is from libSDL - thanks!
