#ifndef LIBENJOY_H
#define LIBENJOY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define LIBENJOY_MAX_JOYSTICK 32

typedef struct libenjoy_joy_info
{
    char *name;
    uint32_t id;
    char opened;
} libenjoy_joy_info;

typedef struct libenjoy_joy_info_list {
    uint32_t count;
    libenjoy_joy_info **list;
} libenjoy_joy_info_list;

typedef struct libenjoy_joystick {
    uint32_t id;
    char valid;
    struct libenjoy_os_specific *os;
} libenjoy_joystick;



void libenjoy_init(void);
void libenjoy_close(void);
void libenjoy_enumerate(void);

libenjoy_joy_info_list *libenjoy_get_info_list(void);
void libenjoy_free_info_list(libenjoy_joy_info_list *list);

libenjoy_joystick *libenjoy_open_joystick(uint32_t id);
void libenjoy_close_joystick(libenjoy_joystick *joy);

int libenjoy_get_axes_num(libenjoy_joystick *joy);
int libenjoy_get_buttons_num(libenjoy_joystick *joy);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_H
