/*
 * \brief  Linux emulation environment specific to this driver
 * \author Stefan Kalkowski
 * \date   2021-08-31
 */

/*
 * Copyright (C) 2021 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2.
 */

#include <lx_emul.h>
#include <linux/pci.h>

enum {
	UHCI_USBLEGSUP          = 0xc0,
	UHCI_USBRES_INTEL       = 0xc4,
	EHCI_SERIAL_BUS_RELEASE = 0x60,
	EHCI_PORT_WAKE          = 0x62,
};


int pci_read_config_byte(const struct pci_dev * dev,int where,u8 * val)
{
	switch (where) {
	case EHCI_SERIAL_BUS_RELEASE:
		*val = 0;
		return 0;
	};
	lx_emul_trace_and_stop(__func__);
}


int pci_read_config_word(const struct pci_dev * dev,int where,u16 * val)
{
	switch (where) {
	case EHCI_PORT_WAKE:
		*val = 0;
		return 0;
	case UHCI_USBLEGSUP:
		/* force the driver to do a full_reset */
		*val = 0xffff;
		return 0;
	};
	lx_emul_trace_and_stop(__func__);
}


int pci_write_config_byte(const struct pci_dev * dev,int where,u8 val)
{
	switch (where) {
	case UHCI_USBRES_INTEL:
		/* do nothing */
		return 0;
	}
	lx_emul_trace_and_stop(__func__);
}


int pci_write_config_word(const struct pci_dev * dev,int where,u16 val)
{
	switch (where) {
	case UHCI_USBLEGSUP:
		/* do nothing */
		return 0;
	}
	lx_emul_trace_and_stop(__func__);
}
