/**********************************************
**    This file is part of libenjoy
**    http://tasssadar.github.com/libenjoy/
**
**    See README and COPYING
***********************************************/

#ifndef LIBENJOY_H
#define LIBENJOY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define LIBENJOY_MAX_JOYSTICK 16 // limited by windows

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

enum libenjoy_event_types
{
    LIBENJOY_EV_AXIS = 0,
    LIBENJOY_EV_BUTTON
};

typedef struct libenjoy_event {
    char type;
    uint32_t joy_id;
    uint16_t part_id;
    int16_t data;
} libenjoy_event;

void libenjoy_init(void);
void libenjoy_close(void);
void libenjoy_enumerate(void);

libenjoy_joy_info_list *libenjoy_get_info_list(void);
void libenjoy_free_info_list(libenjoy_joy_info_list *list);

libenjoy_joystick *libenjoy_open_joystick(uint32_t id);
void libenjoy_close_joystick(libenjoy_joystick *joy);

int libenjoy_get_axes_num(libenjoy_joystick *joy);
int libenjoy_get_buttons_num(libenjoy_joystick *joy);

int libenjoy_poll(libenjoy_event *ev);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_H
