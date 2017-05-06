#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include "machine.h"
#include "6809.h"

FILE *batch_file;

/********************************************************************
 * The kipper1 SBC
 *
 * 32KByte of RAM at $0000
 * 6850 ACIA at      $A000,$A001
 * 16KByte of ROM at $C000
 ********************************************************************/

/* UART-style console. Console input is blocking (but should not be)
 */
uint8_t kipper1_console_read (struct hw_device *dev, unsigned long addr)
{
    //printf("In console_read with addr=0x%08x pc=0x%04x\n", (unsigned int)addr, get_pc());
    unsigned char ch;
    switch (addr) {
    case 0:
        // status bit
        // hardware supports bits [7], [1], [0]
        return 0x03;
    case 1:
        if (batch_file && fread( &ch, 1, 1, batch_file)) {
        }
        else {
            ch = getchar();
        }
        // key conversions to make keyboard look DOS-like
        if (ch == 127) return 8; // rubout->backspace
        if (ch == 10) return 13; // LF->CR
        return ch;
    default:
        printf("In console_read with addr=0x%08x\n", (unsigned int)addr);
        return 0x42;
    }
}

void kipper1_console_write (struct hw_device *dev, unsigned long addr, uint8_t val)
{
    //printf("In console_write with addr=0x%08x val=0x%02x pc=0x%04x\n", (unsigned int)addr, val, get_pc());
    //fprintf(log_file,"%02x~%02x\n",(unsigned char)(addr&0xff),val);
    switch (addr) {
    case 0:
        printf("In console_write with addr=0x%08x val=0x%02x\n",(unsigned int)addr, val);
        break;

    case 1:
        //if ((val != 0x0d) && (val != 0x20) && (val != 0x0a) && (val < '0')) {
        //    printf("Char 0x%02x", val);
        //}
        putchar(val);
        break;

    default:
        printf("In console_write with addr=0x%08x val=0x%02x\n",(unsigned int)addr, val);
    }
}


void kipper1_init (const char *boot_rom_file)
{
    struct hw_device *kipper1_console, *rom;

    /* 32K RAM from 0000 to 7FFF */
    device_define ( ram_create (0x8000), 0,
                    0x0000, 0x8000, MAP_READWRITE );

    /* 16K ROM from C000 to FFFF */
    rom = rom_create (boot_rom_file, 0x4000);
    device_define (rom , 0,
                   0xC000, 0x4000, MAP_READABLE);

    /* I/O console at A000
     *  SCCACMD at 0xA000
     *  SCCADTA at 0xA001
     */

    // flag inch_avail
    // counter outch_busy init 0

    // on read from SCCACMD if inch_avail set bit 0. If outch_busy=0 set bit 2.
    // if outch_busy!=0, increment it module 4 (ie, it counts up to 4 then stops at 0)
    //
    // on read from SCCADTA expect inch_avail to be true else fatal error. Return char.
    //
    // on write to SCCADTA expect outch_busy=0 else fatal error, increment outch_busy (to 1)

    kipper1_console = console_create();
    kipper1_console->class_ptr->read = kipper1_console_read;
    kipper1_console->class_ptr->write = kipper1_console_write;

    device_define ( kipper1_console, 0,
                    0xA000, BUS_MAP_SIZE, MAP_READWRITE );



    /* If a file kipper1.bat exists, supply input from it until
       it's exhausted.
    */
    batch_file = file_open(NULL, "kipper1.bat", "rb");
}


struct machine kipper1_machine =
{
	.name = "kipper1",
	.fault = fault,
	.init = kipper1_init
};
