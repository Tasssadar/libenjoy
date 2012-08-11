#ifndef LIBENJOY_P_H
#define LIBENJOY_P_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern struct libenjoy_joy_info_list joy_info;

void libenjoy_init_private(void);
uint32_t libenjoy_get_new_joyid(void);

int libenjoy_joy_info_created(uint32_t id);
void libenjoy_add_joy_info(libenjoy_joy_info *inf);
void libenjoy_destroy_joy_info(uint32_t id);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_P_H
