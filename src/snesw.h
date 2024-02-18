#ifndef SNESW_H
#define SNESW_H

#include <snes.h>

#define I_RAM_OFFSET ((uint32_t) 0x3000)

void snesw_pads_current(u16 *dst, u16 value);

#endif
