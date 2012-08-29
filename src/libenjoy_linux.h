/**********************************************
**    This file is part of libenjoy
**    https://github.com/Tasssadar/libenjoy
**
**    See README and COPYING
***********************************************/

#ifndef LIBENJOY_LINUX_H
#define LIBENJOY_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/joystick.h>
#include <stdint.h>

typedef struct libenjoy_known_info
{
    uint64_t name_hash;
    uint32_t id;
    char *path;
} libenjoy_known_info;

typedef struct libenjoy_os_specific
{
    int fd;
} libenjoy_os_specific;

typedef struct libenjoy_os_ctx
{
    struct libenjoy_known_info **known_devs;

    uint32_t invalid_reads[LIBENJOY_MAX_JOYSTICK];
    uint8_t invalid_ritr;
    uint8_t invalid_witr;
} libenjoy_os_ctx;

libenjoy_known_info *libenjoy_get_known_hash(libenjoy_os_ctx *octx, uint64_t hash);
libenjoy_known_info *libenjoy_get_known_id(libenjoy_os_ctx *octx, uint32_t id);
libenjoy_known_info *libenjoy_add_known_id(libenjoy_os_ctx *octx, uint64_t hash, char *path);

int libenjoy_set_id_exists(uint32_t id, uint32_t *list, uint32_t size);
uint32_t *libenjoy_create_existing_ids(libenjoy_context *ctx);

void libenjoy_invalid_read_add(libenjoy_os_ctx *octx, uint32_t id);
uint8_t libenjoy_invalid_inc_if_can(uint8_t val);
uint32_t libenjoy_invalid_read_get(libenjoy_os_ctx *octx);
void libenjoy_invalid_read_pop(libenjoy_os_ctx *octx);

uint64_t libenjoy_hash(char *str);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_LINUX_H
