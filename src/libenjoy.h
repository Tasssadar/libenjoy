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
} libenjoy_joy_info;

typedef struct libenjoy_joy_info_list {
    uint32_t count;
    libenjoy_joy_info **list;
} libenjoy_joy_info_list;


void libenjoy_init(void);
void libenjoy_enumerate(void);

libenjoy_joy_info_list *libenjoy_get_info_list(void);
void libenjoy_free_info_list(libenjoy_joy_info_list *list);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_H
