#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include "machine.h"



/*
 * Initialize the simple machine, which is the default
 * machine that has no bells or whistles.
 */
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
