#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"
#include "snesw.h"

static void c_snes_pad_wait_for_scan(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(scanPads, 0);
}

static void *debug_current, *debug_res;
static void c_snes_pad_current(mrbc_vm *vm, mrbc_value v[], int argc) {
  // if (argc != 1) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }
  // if (mrbc_type(v[1]) != MRBC_TT_INTEGER) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }

  u16 res;
  debug_res = &res;
  debug_current = (void *)((uint32_t)&res + I_RAM_OFFSET);
  call_s_cpu(snesw_pads_current, sizeof(void *) + sizeof(u16),
             (void *)(&res + I_RAM_OFFSET), (u16)v[1].i);

  SET_INT_RETURN(res);
}

void snes_init_class_pad(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "Pad", NULL);

  mrbc_define_method(vm, cls, "wait_for_scan", c_snes_pad_wait_for_scan);
  mrbc_define_method(vm, cls, "current", c_snes_pad_current);
}
