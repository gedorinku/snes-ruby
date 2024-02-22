#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"

static void c_snes_oam_set(mrbc_vm *vm, mrbc_value v[], int argc) {
  oamSet(v[1].i, v[2].i, v[3].i, v[4].i, v[5].i, v[6].i, v[7].i, v[8].i);
}

static void c_snes_oam_set_ex(mrbc_vm *vm, mrbc_value v[], int argc) {
  const bool hide = v[3].tt == MRBC_TT_TRUE ? true : false;

  oamSetEx(v[1].i, v[2].i, hide);
}

static void c_snes_oam_show(mrbc_vm *vm, mrbc_value v[], int argc) {
  oamSetVisible(v[1].i, OBJ_SHOW);
}

static void c_snes_oam_hide(mrbc_vm *vm, mrbc_value v[], int argc) {
  oamSetVisible(v[1].i, OBJ_HIDE);
}

void snes_init_class_oam(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "OAM", NULL);

  mrbc_define_method(vm, cls, "set", c_snes_oam_set);
  mrbc_define_method(vm, cls, "set_ex", c_snes_oam_set_ex);
  mrbc_define_method(vm, cls, "show", c_snes_oam_show);
  mrbc_define_method(vm, cls, "hide", c_snes_oam_hide);
}
