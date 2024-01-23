#include <snes.h>

#include <stdio.h>
#include <stdlib.h>
#include "mrubyc.h"

#include "c_snes.h"

#include "main.rb.bytecode.c"

extern char tilfont, palfont;

int run()
{
    mrbc_init_global();
    mrbc_init_class();

    mrbc_vm *vm = mrbc_vm_open(NULL);
    if (vm == NULL)
    {
        consoleDrawText(10, 10, "vm is null");
        return -1;
    }

    snes_init_class_snes(vm);

    if (mrbc_load_mrb(vm, mrbbuf) != 0)
    {
        consoleDrawText(10, 10, "failed to load");
        return -1;
    }

    mrbc_vm_begin(vm);
    int ret = mrbc_vm_run(vm);
    mrbc_vm_end(vm);
    mrbc_vm_close(vm);

    return ret;
}

int main(void)
{
    // Initialize SNES
    consoleInit();

    // Initialize text console with our font
    consoleSetTextVramBGAdr(0x6800);
    consoleSetTextVramAdr(0x3000);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);

    // Init background
    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    // Now Put in 16 color mode and disable Bgs except current
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    // Wait for nothing :P
    setScreenOn();

    /*
        start mruby/c
    */
    int ret = run();
    char res[16];
    sprintf(res, "ret: %d", ret);
    consoleDrawText(10, 10, res);

    while (1)
    {
        WaitForVBlank();
    }
    return 0;
}
