#include "config.h"
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
#error
#endif

#include <fcntl.h>

#include "machine.h"
#include "mmu.h"
#include "ioexpand.h"
#include "serial.h"
#include "imux.h"
#include "timer.h"



/*
 * Initialize the second-generation EON machine (EON2).
 */
void eon2_init (const char *boot_rom_file)
{
	struct hw_device *dev, *ram_dev, *mmudev, *iodev, *intdev;

	/* Create a 1MB RAM */
	ram_dev = ram_create (0x100000);

	/* Place the RAM behind a small MMU, which dynamically remaps
	portions of the RAM into the processor address space */
	mmudev = small_mmu_create (ram_dev);

	/* Create and map in the ROM */
	dev = rom_create (boot_rom_file, 0x800);
	device_define (dev, 0, 0xF800, 0x0800, MAP_READABLE);

	/* Create an I/O expander to hold all of the I/O registers.
	Each device is allocated only 8 bytes. */
	iodev = ioexpand_create ();
	device_define (iodev, 0, 0xFF00, 128, MAP_READWRITE);
	ioexpand_attach (iodev, 0, 0, serial_create ());
	ioexpand_attach (iodev, 1, 0, disk_create ("disk.bin", ram_dev));
	ioexpand_attach (iodev, 2, 0, mmudev);
	ioexpand_attach (iodev, 3, 0, intdev = imux_create (1));
	/* 4 = config EEPROM */
	/* 5 = video display */
	/* 6 = battery-backed clock */
	/* 7 = power control (reboot/off) */
	/* 8 = periodic timer/oscillator */
	/* 9 = hostfile (for debug only) */
	ioexpand_attach (iodev, 9, 0, hostfile_create ("hostfile", O_RDWR));
	/* etc. up to device 15 */
	/* May need to define an I/O _multiplexer_ to support more than 16 devices */

	dev = oscillator_create (intdev, 0);
}


struct machine eon2_machine =
{
	.name = "eon2",
	.fault = fault,
	.init = eon2_init,
	.periodic = 0,
};

