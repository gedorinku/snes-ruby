#include <snes.h>

#include "mrubyc.h"
#include "c_snes/c_console.h"

static void c_snes_wait_for_vblank(mrbc_vm *vm, mrbc_value v[], int argc)
{
  WaitForVBlank();
}

void snes_init_class_snes(struct VM *vm)
{
  mrbc_class *cls = mrbc_define_class(vm, "SNES", NULL);

  mrbc_define_method(vm, cls, "wait_for_vblank", c_snes_wait_for_vblank);

  snes_init_class_console(vm, cls);
}
