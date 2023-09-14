/**
 * \brief  Basic types for USB (C/C++ compatible)
 * \author Sebastian Sumpf
 * \author Stefan Kalkowski
 * \date   2014-12-08
 */

/*
 * Copyright (C) 2014-2023 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__USB__TYPES_H_
#define _INCLUDE__USB__TYPES_H_

#include <base/fixed_stdint.h>

#ifdef __cplusplus
namespace Usb {

	enum Endpoint_type {
		USB_ENDPOINT_BULK      = 0x2,
		USB_ENDPOINT_INTERRUPT = 0x3,
	};

	/**
	 * The following three enums are ORed together to form a control request
	 * type
	 */
	enum Endpoint_direction {
		USB_ENDPOINT_OUT       = 0,
		USB_ENDPOINT_IN        = 0x80,
	};

	enum Request_type {
		USB_TYPE_STANDARD = 0,
		USB_TYPE_CLASS    = (1U << 5),
		USB_TYPE_VENDOR   = (2U << 5),
		USB_TYPE_RESERVED = (3U << 5),
	};

	enum Recipient {
		USB_RECIPIENT_DEVICE    = 0,
		USB_RECIPIENT_INTERFACE = 0x1,
		USB_RECIPIENT_ENDPOINT  = 0x2,
		USB_RECIPIENT_OTHER     = 0x3,
	};
}
#endif /* __cplusplus */


struct genode_usb_isoc_descriptor
{
	genode_uint32_t size;
	genode_uint32_t actual_size;
} __attribute__((packed));


struct genode_usb_isoc_transfer_header
{
	genode_uint32_t                   number_of_packets;
	struct genode_usb_isoc_descriptor packets[0];
} __attribute__((packed));


/**
 * USB hardware device descriptor
 */
struct genode_usb_device_descriptor
{
	genode_uint8_t  length;
	genode_uint8_t  type;
	genode_uint16_t usb;
	genode_uint8_t  dclass;
	genode_uint8_t  dsubclass;
	genode_uint8_t  dprotocol;
	genode_uint8_t  max_packet_size;
	genode_uint16_t vendor_id;
	genode_uint16_t product_id;
	genode_uint16_t device_release;
	genode_uint8_t  manufacturer_index;
	genode_uint8_t  product_index;
	genode_uint8_t  serial_number_index;
	genode_uint8_t  num_configs;
} __attribute__((packed));


/**
 * USB hardware configuration descriptor
 */
struct genode_usb_config_descriptor
{
	genode_uint8_t  length;
	genode_uint8_t  type;
	genode_uint16_t total_length;
	genode_uint8_t  num_interfaces;
	genode_uint8_t  config_value;
	genode_uint8_t  config_index;
	genode_uint8_t  attributes;
	genode_uint8_t  max_power;
} __attribute__((packed));


/**
 * USB hardware interface descriptor
 */
struct genode_usb_interface_descriptor
{
	genode_uint8_t length;
	genode_uint8_t type;
	genode_uint8_t number;
	genode_uint8_t alt_settings;
	genode_uint8_t num_endpoints;
	genode_uint8_t iclass;
	genode_uint8_t isubclass;
	genode_uint8_t iprotocol;
	genode_uint8_t interface_index;
} __attribute__((packed));


/**
 * USB hardware endpoint descriptor
 */
struct genode_usb_endpoint_descriptor
{
	genode_uint8_t  length;
	genode_uint8_t  type;
	genode_uint8_t  address;
	genode_uint8_t  attributes;
	genode_uint16_t max_packet_size;
	genode_uint8_t  polling_interval;
} __attribute__((packed));

#endif /* _INCLUDE__USB__TYPES_H_ */
