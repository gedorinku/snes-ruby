#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"

static void c_snes_bg_scroll(mrbc_vm *vm, mrbc_value v[], int argc) {
  // if (argc != 3) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }
  // if (mrbc_type(v[1]) != MRBC_TT_INTEGER ||
  //     mrbc_type(v[2]) != MRBC_TT_INTEGER ||
  //     mrbc_type(v[3]) != MRBC_TT_INTEGER) {
  //   mrbc_raise(vm, MRBC_CLASS(ArgumentError), NULL);
  //   return;
  // }

  bgSetScroll(v[1].i, v[2].i, v[3].i);
}

void snes_init_class_bg(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "Bg", NULL);
  mrbc_define_method(vm, cls, "scroll", c_snes_bg_scroll);
}
