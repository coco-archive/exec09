// See eon.h for a description of this machine

#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include "machine.h"
#include "eon.h"


/**
 * Initialize the EON machine.
 */
void eon_init (const char *boot_rom_file)
{
	struct hw_device *dev, *ram_dev;

	/* The MMU must be defined first, as all other devices
	that are attached can try to hook into the MMU. */
	device_define ( mmu_create (), 0,
		MMU_ADDR, BUS_MAP_SIZE, MAP_READWRITE+MAP_FIXED );

	/* A 1MB RAM part is mapped into all of the allowable 64KB
	address space, until overriden by other devices. */
	device_define ( ram_dev = ram_create (RAM_SIZE), 0,
		0x0000, MAX_CPU_ADDR, MAP_READWRITE );

	device_define ( rom_create (boot_rom_file, BOOT_ROM_SIZE), 0,
		BOOT_ROM_ADDR, BOOT_ROM_SIZE, MAP_READABLE );

	device_define ( dev = console_create (), 0,
		CONSOLE_ADDR, BUS_MAP_SIZE, MAP_READWRITE );
	device_define (dev, 0,
		0xFF00, BUS_MAP_SIZE, MAP_READWRITE );

	device_define ( disk_create ("disk.bin", ram_dev), 0,
		DISK_ADDR(0), BUS_MAP_SIZE, MAP_READWRITE);
}


struct machine eon_machine =
{
	.name = "eon",
	.fault = fault,
	.init = eon_init,
	.periodic = 0,
};
