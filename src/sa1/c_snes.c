#include <snes.h>

#include "c_snes/c_bg.h"
#include "c_snes/c_console.h"
#include "c_snes/c_oam.h"
#include "c_snes/c_pad.h"
#include "c_snes/c_spc.h"
#include "sa1/mrubyc/mrubyc.h"

static void c_snes_wait_for_vblank(mrbc_vm *vm, mrbc_value v[], int argc) {
  // call_s_cpu(WaitForVBlank, 0);
}

static void c_snes_rand(mrbc_vm *vm, mrbc_value v[], int argc) {
  SET_INT_RETURN(rand() % v[1].i);
}

void snes_init_class_snes(struct VM *vm) {
  mrbc_class *cls = mrbc_define_class(vm, "SNES", NULL);

  mrbc_define_method(vm, cls, "wait_for_vblank", c_snes_wait_for_vblank);
  mrbc_define_method(vm, cls, "rand", c_snes_rand);

  snes_init_class_bg(vm, cls);
  snes_init_class_console(vm, cls);
  snes_init_class_oam(vm, cls);
  snes_init_class_pad(vm, cls);
  snes_init_class_spc(vm, cls);
}
