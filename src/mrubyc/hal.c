#include <string.h>
#include "hal.h"

#define HAL_BUF_SIZE (256)
static char hal_buf[HAL_BUF_SIZE];
static int hal_buf_pos = 0;

int hal_write(int fd, const void *buf, int nbytes) {
    int l = 0;
    while (l < nbytes) {
        int s = HAL_BUF_SIZE - hal_buf_pos;
        if ((nbytes - l) < s) {
            s = (nbytes - l);
        }

        memset(hal_buf + hal_buf_pos, 0, s);
        memcpy(hal_buf, (const char *)buf + l, s);

        l += s;
        hal_buf_pos += s;

        if (HAL_BUF_SIZE <= hal_buf_pos) {
            hal_buf_pos = 0;
        }
    }

    return 0;
}
