#ifndef LIBENJOY_P_H
#define LIBENJOY_P_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern struct libenjoy_joy_info_list joy_info;

typedef struct libenjoy_os_specific libenjoy_os_specific;
typedef struct libenjoy_joystick libenjoy_joystick;

typedef struct libenjoy_joystick_list {
    uint32_t count;
    struct libenjoy_joystick **list;
} libenjoy_joystick_list;

extern struct libenjoy_joystick_list joy_list;

void libenjoy_init_private(void);
void libenjoy_close_private(void);
uint32_t libenjoy_get_new_joyid(void);

int libenjoy_joy_info_created(uint32_t id);
void libenjoy_add_joy_info(libenjoy_joy_info *inf);
void libenjoy_destroy_joy_info(uint32_t id);
libenjoy_joy_info* libenjoy_get_joy_info(uint32_t id);

libenjoy_os_specific *libenjoy_open_os_specific(uint32_t id);
void libenjoy_close_os_specific(libenjoy_os_specific *os);

void libenjoy_add_joy_to_list(libenjoy_joystick *joy);
void libenjoy_rm_joy_from_list(libenjoy_joystick *joy);
void libenjoy_joy_set_valid(uint32_t id, char valid);
libenjoy_joystick *libenjoy_get_joystick(uint32_t id);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_P_H
