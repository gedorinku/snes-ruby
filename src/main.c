#include <snes.h>
#include <stdio.h>
#include <stdlib.h>

#include "c_snes.h"
#include "main.rb.bytecode.c"
#include "mrubyc.h"

extern char patterns, patterns_end;
extern char palette;
extern char map, map_end;

int run() {
  mrbc_init_global();
  mrbc_init_class();

  mrbc_vm *vm = mrbc_vm_open(NULL);
  if (vm == NULL) {
    return -1;
  }

  snes_init_class_snes(vm);

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

int main(void) {
  consoleInit();

  bgInitTileSet(1, &patterns, &palette, 0, (&patterns_end - &patterns), 16 * 2,
                BG_16COLORS, 0x4000);

  bgInitMapSet(1, &map, (&map_end - &map), SC_64x64, 0x1000);

  bgSetGfxPtr(0, 0x2000);
  bgSetMapPtr(0, 0x6800, SC_32x32);

  setMode(BG_MODE1, 0);
  bgSetDisable(0);
  bgSetDisable(2);
  setScreenOn();

  ret = run();

  while (1) {
    WaitForVBlank();
  }
  return 0;
}
