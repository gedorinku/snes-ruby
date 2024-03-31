#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"

void snes_init_class_bg(struct VM *vm, mrbc_class *snes_class);
void snes_bg_set_default_tile_map(int bg, const u8 *map, size_t size,
                                  u16 vram_addr);
