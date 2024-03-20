#include <snes.h>

#include "sa1/mrubyc/mrubyc.h"
#include "snesw.h"

static void c_snes_spc_process(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(spcProcess, 0);
}

static void c_snes_spc_play_sound(mrbc_vm *vm, mrbc_value v[], int argc) {
  call_s_cpu(spcPlaySound, sizeof(u8) * 1, (u8)v[1].i);
}

void snes_init_class_spc(struct VM *vm, mrbc_class *snes_class) {
  mrbc_class *cls = mrbc_define_class_under(vm, snes_class, "SPC", NULL);

  mrbc_define_method(vm, cls, "process", c_snes_spc_process);
  mrbc_define_method(vm, cls, "play_sound", c_snes_spc_play_sound);
}
