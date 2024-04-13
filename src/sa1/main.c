#include "bg.h"
#include "c_snes.h"
#include "c_snes/c_bg.h"
#include "main.rb.bytecode.c"
#include "mrubyc/mrubyc.h"

extern u8 tiles_map, tiles_map_end;

int run() {
  mrbc_init_global();
  mrbc_init_class();

  mrbc_vm *vm = mrbc_vm_open(NULL);
  if (vm == NULL) {
    return -1;
  }

  snes_init_class_snes(vm);
  snes_bg_set_default_tile_map(1, &tiles_map, (&tiles_map_end - &tiles_map),
                               SNES_BG_TILE_MAP_VRAM_ADDR);

  if (mrbc_load_mrb(vm, mrbbuf) != 0) {
    return -1;
  }

  mrbc_vm_begin(vm);
  int ret = mrbc_vm_run(vm);
  mrbc_vm_end(vm);
  mrbc_vm_close(vm);

  return ret;
}

volatile int ret;

int sa1_main() {
  ret = run();

  while (1) {
  }
}
