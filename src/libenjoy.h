/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
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
#define LIBENJOY_EVENT_BUFF_SIZE 128

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
    uint8_t num_axes;
    uint8_t num_buttons;
    struct libenjoy_os_specific *os;
    struct libenjoy_context *ctx;
} libenjoy_joystick;

enum libenjoy_event_types
{
    LIBENJOY_EV_AXIS = 0,
    LIBENJOY_EV_BUTTON,
    LIBENJOY_EV_CONNECTED,
};

typedef struct libenjoy_event {
    uint32_t joy_id;
    int16_t data;
    uint8_t part_id;
    uint8_t type;
} libenjoy_event;

typedef struct libenjoy_joystick_list {
    uint32_t count;
    struct libenjoy_joystick **list;
} libenjoy_joystick_list;

typedef struct libenjoy_context
{
    struct libenjoy_joy_info_list info_list;
    struct libenjoy_joystick_list joy_list;

    struct libenjoy_event event_buffer[LIBENJOY_EVENT_BUFF_SIZE];
    uint16_t buff_wr_itr;
    uint16_t buff_rd_itr;

    struct libenjoy_os_ctx *os;

    struct libenjoy_joy_change_ev **change_events;
} libenjoy_context;

struct libenjoy_context *libenjoy_init(void);
void libenjoy_close(libenjoy_context *ctx);
void libenjoy_enumerate(libenjoy_context *ctx);

libenjoy_joy_info_list *libenjoy_get_info_list(libenjoy_context *ctx);
void libenjoy_free_info_list(libenjoy_joy_info_list *list);

libenjoy_joystick *libenjoy_open_joystick(libenjoy_context *ctx, uint32_t id);
void libenjoy_close_joystick(libenjoy_joystick *joy);

int libenjoy_get_axes_num(libenjoy_joystick *joy);
int libenjoy_get_buttons_num(libenjoy_joystick *joy);

int libenjoy_poll(libenjoy_context *ctx, libenjoy_event *ev);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_H
