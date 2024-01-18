#include "mem_kwd.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "int8.h"
#include "mrubyc.h"

#include "mrb_bytecode.c"

int run()
{
    mrbc_init_global();
    mrbc_init_class();

    mrbc_vm *vm = mrbc_vm_open(NULL);
    if (vm == NULL)
    {
        return -1;
    }

    if (mrbc_load_mrb(vm, mrbbuf) != 0)
    {
        return -1;
    }

    mrbc_vm_begin(vm);
    int ret = mrbc_vm_run(vm);
    mrbc_vm_end(vm);
    mrbc_vm_close(vm);

    return ret;
}

static int ret = 0;

int main(void)
{
    ret = run();

    while (1)
    {
    }

    return 0;
}

void NmiHandler()
{

}

void IrqHandler()
{

}
