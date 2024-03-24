#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"

static void c_snes_oam_set(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(oamSet,
             sizeof(u16) * 3 + sizeof(u8) * 3 + sizeof(u16) + sizeof(u8),
             (u16)v[1].i, (u16)v[2].i, (u16)v[3].i, (u8)v[4].i, (u8)v[5].i,
             (u8)v[6].i, (u16)v[7].i, (u8)v[8].i);
}

static void c_snes_oam_set_ex(mrbc_vm *vm, mrbc_value v[], int argc) {
  const bool hide = v[3].tt == MRBC_TT_TRUE ? true : false;

  call_s_cpu(oamSetEx, sizeof(u16) + sizeof(u8) * 2, (u16)v[1].i, (u8)v[2].i,
             (u8)hide);
}

static void c_snes_oam_show(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(oamSetVisible, sizeof(u16) + sizeof(u8), (u16)v[1].i,
             (u8)OBJ_SHOW);
}

static void c_snes_oam_hide(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(oamSetVisible, sizeof(u16) + sizeof(u8), (u16)v[1].i,
             (u8)OBJ_HIDE);
}

void snes_init_class_oam(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "OAM", NULL);

  mrbc_define_method(vm, cls, "set", c_snes_oam_set);
  mrbc_define_method(vm, cls, "set_ex", c_snes_oam_set_ex);
  mrbc_define_method(vm, cls, "show", c_snes_oam_show);
  mrbc_define_method(vm, cls, "hide", c_snes_oam_hide);
}
