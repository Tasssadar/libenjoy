/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
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
#define HATS_COUNT 2
#define AXES_HATS_COUNT AXES_COUNT+HATS_COUNT
#define BUTTONS_COUNT 32
#define AXIS_MIN	-32768
#define AXIS_MAX	32767
#define JOY_AXIS_THRESHOLD      (((AXIS_MAX)-(AXIS_MIN))/256)

typedef struct libenjoy_os_specific {
    UINT sys_id;
    int axes_offset[AXES_COUNT];
    float axes_scale[AXES_COUNT];
    int16_t axes[AXES_HATS_COUNT];
    char buttons[BUTTONS_COUNT];
} libenjoy_os_specific;

typedef struct libenjoy_os_ctx {
    libenjoy_known_info **known_devs;
} libenjoy_os_ctx;

libenjoy_known_info *libenjoy_get_known_dev(libenjoy_os_ctx *octx, uint32_t guid);
libenjoy_known_info *libenjoy_get_known_dev_by_id(libenjoy_os_ctx *octx, uint32_t id);
libenjoy_known_info *libenjoy_add_known_dev(libenjoy_os_ctx *octx, uint32_t guid, UINT sys_id);

int libenjoy_get_joy_name(int index, const char *szRegKey, char *res);

int libenjoy_set_id_exists(uint32_t id, uint32_t *list, int size);
uint32_t *libenjoy_create_existing_ids(libenjoy_context *ctx);

void libenjoy_translate_pov(DWORD pov, int16_t *ax1,  int16_t *ax2);
void libenjoy_send_hat_event(libenjoy_joystick *joy, int idx, int16_t val);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_WIN32_H
