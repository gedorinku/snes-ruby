#include "c_snes.h"
#include "main.rb.bytecode.c"
#include "mrubyc/mrubyc.h"

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

int sa1_main() {
  ret = run();

  while (1) {
  }
}
