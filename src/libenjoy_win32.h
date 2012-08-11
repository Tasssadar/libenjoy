/**********************************************
**    This file is part of libenjoy
**    http://tasssadar.github.com/libenjoy/
**
**    See README and COPYING
***********************************************/

#ifndef LIBENJOY_WIN32_H
#define LIBENJOY_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct libenjoy_known_info {
    uint32_t id;
    uint32_t guid; // ((mId << 16) | pid)
    UINT sys_id; // 0-16
} libenjoy_known_info;

#define AXES_COUNT 6
#define BUTTONS_COUNT 32
#define AXIS_MIN	-32768
#define AXIS_MAX	32767
#define JOY_AXIS_THRESHOLD      (((AXIS_MAX)-(AXIS_MIN))/256)

typedef struct libenjoy_os_specific {
    UINT sys_id;
    uint8_t num_axes;
    uint8_t num_buttons;
    int axes_offset[AXES_COUNT];
    float axes_scale[AXES_COUNT];
    int16_t axes[AXES_COUNT];
    char buttons[BUTTONS_COUNT];
} libenjoy_os_specific;

libenjoy_known_info *libenjoy_get_known_dev(uint32_t guid);
libenjoy_known_info *libenjoy_get_known_dev_by_id(uint32_t id);
libenjoy_known_info *libenjoy_add_known_dev(uint32_t guid, UINT sys_id);

int libenjoy_get_joy_name(int index, const char *szRegKey, char *res);

void libenjoy_set_id_exists(uint32_t id, uint32_t *list, int size);
uint32_t *libenjoy_create_existing_ids(void);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_WIN32_H
