#include <snes.h>

void snesw_pads_current(u16 *dst, u16 value) { *dst = padsCurrent(value); }

void snesw_wait_and_dma_to_vram(const u8 *source, u16 address, u16 size) {
  // DMAはVBlank中でないと動作しない
  WaitForVBlank();

  // TODO: VBlankを待ってるので多分DMA使わずにコピーしたほうが高速
  dmaCopyVram(source, address, size);
}
