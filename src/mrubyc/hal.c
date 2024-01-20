#include <string.h>
#include "hal.h"

#define HAL_BUF_SIZE (1024)
static char hal_write_buf[HAL_BUF_SIZE];
static int hal_write_buf_pos = 0;

int hal_write(int fd, const void *buf, int nbytes) {
    int l = 0;
    while (l < nbytes) {
        int size = HAL_BUF_SIZE - hal_write_buf_pos;
        if (nbytes - l < size) {
            size = nbytes - l;
        }

        memcpy(hal_write_buf + hal_write_buf_pos, (const char *)buf + l, size);

        l += size;
        hal_write_buf_pos += size;

        if (HAL_BUF_SIZE <= hal_write_buf_pos) {
            hal_write_buf_pos = 0;
        }
    }

    return nbytes;
}
