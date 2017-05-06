/**************************************************************

  The simple machine is the default machine that has no
  bells or whistles.


  It has full 64KB of RAM, minus some input/output functions
  mapped at $FF00 and $FF01.

  If you compile a program with gcc6809 with no special linker
  option, you'll get an S-record file that is suitable for running
  on this machine.  The S-record file will include a vector table
  at $FFF0, with a reset vector that points to a _start function,
  which will call your main() function.

  When main returns, _start writes to an 'exit register' at $FF01,
  which the simulator interprets and causes it to stop.

  64 KB RAM
  $FF00 console I/O
  $FF01 a write to this address terminates the emulation

*******************************************************************/

#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include "machine.h"


void simple_init (const char *boot_rom_file)
{
        device_define ( ram_create (MAX_CPU_ADDR), 0, 0x0000, MAX_CPU_ADDR, MAP_READWRITE );
        device_define ( console_create (),         0, 0xFF00, BUS_MAP_SIZE, MAP_READWRITE );
}


struct machine simple_machine =
{
	.name = "simple",
	.fault = fault,
	.init = simple_init,
	.periodic = 0,
};
