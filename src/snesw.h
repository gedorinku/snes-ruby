#ifndef SNESW_H
#define SNESW_H

#include <snes.h>

void snesw_pads_current(u16 *dst, u16 value);
void snesw_wait_and_dma_to_vram(const u8 *source, u16 address, u16 size);

#endif
