#include "mem_kwd.h"

// #include <snes.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "int8.h"
#include "mrubyc.h"

#include "main.rb.bytecode.h"

// #define MRBC_MEMORY_SIZE (40960)
#define MRBC_MEMORY_SIZE (0xc000)
static uint8_t *ruby_memory_pool;

extern uint8_t tilfont, palfont;

int run()
{
    mrbc_init_alloc(ruby_memory_pool, MRBC_MEMORY_SIZE);
    mrbc_init_global();
    mrbc_init_class();

    mrbc_vm *vm = mrbc_vm_open(NULL);
    if (vm == NULL)
    {
        // consoleDrawText(10, 10, "vm is null");
        return 123;
    }

    if (mrbc_load_mrb(vm, mrbbuf) != 0)
    {
        // consoleDrawText(10, 10, "failed to load");
        return 456;
    }

    mrbc_vm_begin(vm);
    int ret = mrbc_vm_run(vm);
    mrbc_vm_end(vm);
    mrbc_vm_close(vm);

    return ret;
}
volatile static int count = 0;
volatile static int ret = 0;

int main(void)
{
    ruby_memory_pool = malloc(MRBC_MEMORY_SIZE * sizeof(uint8_t));
    // // Initialize SNES
    // consoleInit();

    // // Initialize text console with our font
    // consoleSetTextVramBGAdr(0x6800);
    // consoleSetTextVramAdr(0x3000);
    // consoleSetTextOffset(0x0100);
    // consoleInitText(0, 16 * 2, &tilfont, &palfont);

    // // Init background
    // bgSetGfxPtr(0, 0x2000);
    // bgSetMapPtr(0, 0x6800, SC_32x32);

    // // Now Put in 16 color mode and disable Bgs except current
    // setMode(BG_MODE1, 0);
    // bgSetDisable(1);
    // bgSetDisable(2);

    // /*
    //     start mruby/c
    // */
    // int ret;
    ret = run();
    // char res[16];
    // sprintf(res, "ret: %d", ret);
    // consoleDrawText(10, 10, res);

    // consoleDrawText(6, 14, "Hello World !");

    // // Wait for nothing :P
    // setScreenOn();

    while (1)
    {
        count++;
        // WaitForVBlank();
    }
    return 0;
}

void far NmiHandler()
{

}

void far IrqHandler()
{

}
