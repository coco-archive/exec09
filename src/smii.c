/********************************************************************
 * The Scroungemaster II machine, a platform
 * for 6809 CamelForth. See
 * Brad Rodriguez http://www.camelforth.com/page.php?6
 * and
 * http://www.bradrodriguez.com/papers/impov3.htm
 ********************************************************************/

#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include "machine.h"
#include "6809.h"

FILE *batch_file;

// for smii console
int smii_i_avail = 1;
int smii_o_busy = 0;


// by inspection command read (should be address 0x7c02) comes in with addr=0x8d
// TODO no way to check for "char available" and so smii_i_busy is always true and
// console input is blocking.
uint8_t smii_console_read (struct hw_device *dev, unsigned long addr)
{
    (void) dev;     // silence unused warning

    unsigned char ch;
    switch (addr) {
    case 0x02: // SCCACMD
        // on output make it seem busy for several polls
        smii_o_busy = smii_o_busy == 0 ? 0 : (smii_o_busy + 1)%4;
        //        printf("02 smii_o_busy = %d return 0x%02x\n",smii_o_busy,(smii_i_avail & 1) | (smii_o_busy == 0 ? 4 : 0));
        return (smii_i_avail & 1) | (smii_o_busy == 0 ? 4 : 0);
    case 0x03: // SCCADTA
        if (batch_file && fread( &ch, 1, 1, batch_file)) {
        }
        else {
            ch = getchar();
        }
        // key conversions to make keyboard look DOS-like
        if (ch == 127) ch = 8; //backspace
        if (ch == 10) ch = 13; //cr
        return ch;
    default:
        printf("In smii_console_read with addr=0x%08x\n", (unsigned int)addr);
        return 0x42;
    }
}


void smii_console_write (struct hw_device *dev, unsigned long addr, uint8_t val)
{
    (void) dev;     // silence unused warning

    switch (addr) {
    case 0x00:
    case 0x02:
        // UART setup. Not emulated; just ignore it.
        break;
    case 0x03: // SCCADTA
        if (smii_o_busy != 0) printf("Oops! Write to busy UART\n");
        smii_o_busy = 1;
        putchar(val);
        break;
    default:
        printf("In smii_console_write with addr=0x%08x val=0x%02x\n",(unsigned int)addr, val);
    }
}


void smii_init (const char *boot_rom_file)
{
    struct hw_device *smii_console, *rom, *quiet;

    /* RAM from 0 to 7BFF */
    device_define ( ram_create (0x7C00), 0,
                    0x0000, 0x7C00, MAP_READWRITE );

    /* ROM from E000 to FFFF */
    rom = rom_create (boot_rom_file, 0x2000);
    device_define (rom , 0,
                   0xE000, 0x2000, MAP_READABLE);

    /* The address space 8000-DFFF provides aliases of the ROM
       There is write-only mapping logic for 8 RAM pages and this
       is usually accessed by writes to addresses 8000,9000..F000

       The CamelForth image does writes to those addresses at
       in order to initialise the mapping hardware, but makes
       no further use of it. Since the model is strict, the ROM
       is read-only and the write causes a trap.

       To avoid the trap, create 1-page dummy devices at each
       location in order to silently ignore the writes.

       For locations that alias to ROM, change the attribute so
       that writes are ignored.
    */
    quiet = null_create();
    device_define(quiet, 0,    0x8000, BUS_MAP_SIZE, MAP_IGNOREWRITE);
    device_define(quiet, 0,    0x9000, BUS_MAP_SIZE, MAP_IGNOREWRITE);
    device_define(quiet, 0,    0xA000, BUS_MAP_SIZE, MAP_IGNOREWRITE);
    device_define(quiet, 0,    0xB000, BUS_MAP_SIZE, MAP_IGNOREWRITE);
    device_define(quiet, 0,    0xC000, BUS_MAP_SIZE, MAP_IGNOREWRITE);
    device_define(quiet, 0,    0xD000, BUS_MAP_SIZE, MAP_IGNOREWRITE);

    device_define(rom, 0x0000, 0xE000, BUS_MAP_SIZE, MAP_IGNOREWRITE | MAP_READABLE);
    device_define(rom, 0x1000, 0xF000, BUS_MAP_SIZE, MAP_IGNOREWRITE | MAP_READABLE);


    /* Make debug output more informative */
    // ?? haven't seen this work yet..
    sym_add(&internal_symtab, "SCCACMD", to_absolute(0x7c02), 0);
    sym_add(&internal_symtab, "SCCADTA", to_absolute(0x7c03), 0);

    /* I/O console at 7C00
     *  SCCACMD at 0x7C02
     *  SCCADTA at 0x7C03
     */

    // flag inch_avail
    // counter outch_busy init 0

    // on read from SCCACMD if inch_avail set bit 0. If outch_busy=0 set bit 2.
    // if outch_busy!=0, increment it module 4 (ie, it counts up to 4 then stops at 0)
    //
    // on read from SCCADTA expect inch_avail to be true else fatal error. Return char.
    //
    // on write to SCCADTA expect outch_busy=0 else fatal error, increment outch_busy (to 1)

    // need to mimic the hardware that is controlled like this:
    // CODE KEY    \ -- c    get char from serial port
    //    6 # ( D) PSHS,   BEGIN,   SCCACMD LDB,   1 # ANDB,  NE UNTIL,
    //    SCCADTA LDB,   CLRA,   NEXT ;C
    //
    // CODE KEY?   \ -- f    return true if char waiting
    //    6 # ( D) PSHS,   CLRA,   SCCACMD LDB,   1 # ANDB,
    //    NE IF,   -1 # LDB,   THEN,   NEXT ;C
    //
    // CODE EMIT   \ c --    output character to serial port
    //    BEGIN,   SCCACMD LDA,   4 # ANDA,   NE UNTIL,
    //    SCCADTA STB,   6 # ( D) PULS,   NEXT ;C

    smii_console = console_create();
    smii_console->class_ptr->read = smii_console_read;
    smii_console->class_ptr->write = smii_console_write;

    device_define ( smii_console, 0,
                    0x7C00, BUS_MAP_SIZE, MAP_READWRITE );



    /* If a file smii.bat exists, supply input from it until
       it's exhausted.
    */
    batch_file = file_open(NULL, "smii.bat", "rb");
}


struct machine smii_machine =
{
	.name = "smii",
	.fault = fault,
	.init = smii_init
};
