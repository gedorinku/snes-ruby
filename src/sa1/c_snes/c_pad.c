#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"

static void c_snes_pad_wait_for_scan(mrbc_vm *vm, mrbc_value v[], int argc) {
  scanPads();
}

static void c_snes_pad_current(mrbc_vm *vm, mrbc_value v[], int argc) {
  // if (argc != 1) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }
  // if (mrbc_type(v[1]) != MRBC_TT_INTEGER) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }

  SET_INT_RETURN(padsCurrent(v[1].i));
}

void snes_init_class_pad(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "Pad", NULL);

  mrbc_define_method(vm, cls, "wait_for_scan", c_snes_pad_wait_for_scan);
  mrbc_define_method(vm, cls, "current", c_snes_pad_current);
}
