#ifndef LIBENJOY_LINUX_H
#define LIBENJOY_LINUX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/joystick.h>
#include <stdint.h>

typedef struct libenjoy_known_info
{
    dev_t devid;
    uint32_t id;
    char *path;
} libenjoy_known_info;

libenjoy_known_info *libenjoy_get_known_id(dev_t devid);
libenjoy_known_info *libenjoy_add_known_id(dev_t devid, char *path);

void libenjoy_set_id_exists(uint32_t id, uint32_t *list);
uint32_t *libenjoy_create_existing_ids(void);

#ifdef __cplusplus
}
#endif

#endif // LIBENJOY_LINUX_H
