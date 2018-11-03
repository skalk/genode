/*
 * \brief  VDI file as a Block session
 * \author Josef Soentgen
 * \date   2018-11-01
 *
 * Structures are based on the information available in VDICore.h from
 * VirtualBox 5.1.xxx.
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _VDI_TYPES_H_
#define _VDI_TYPES_H_

/* Genode includes */
#include <base/stdint.h>


using ssize_t  = signed long;
using uint8_t  = Genode::uint8_t;
using uint16_t = Genode::uint16_t;
using uint32_t = Genode::uint32_t;
using uint64_t = Genode::uint64_t;

union Random_uuid
{
	uint8_t  au8[16];
	uint16_t au16[8];
	uint32_t au32[4];
	uint64_t au64[2];

	struct
	{
		uint32_t time_low;
		uint16_t time_mid;
		uint16_t time_hi_and_version;
		uint8_t  clock_seq_hi_and_reserved;
		uint8_t  clock_seq_low;
		uint8_t  node[6];
	} dce;

	static Random_uuid generate()
	{
		Random_uuid uuid { };
		uuid.au64[0] = 0x1122334455667788;
		uuid.au64[1] = 0x8877665544332211;

		uuid.dce.time_hi_and_version       &= (0x0fff | 0x4000);
		uuid.dce.clock_seq_hi_and_reserved &= (0x3f | 0x80);
		return uuid;
	}

	bool valid() const { return dce.time_low && dce.time_hi_and_version; }
};


struct Disk_geometry
{
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors;
	enum { SECTOR_SIZE = 512, };
	uint32_t sector_size;
} __attribute__((packed));


struct Preheader
{
	char     info[64];
	enum { SIGNATURE = 0xbeda107fu, };
	uint32_t signature;
	uint32_t version;

	uint16_t major() const { return (version >> 16); }
	uint16_t minor() const { return version; }
	bool     valid() const { return signature == SIGNATURE; }
} __attribute__((packed));


struct HeaderV1Plus
{
	uint32_t size;
	enum {
		TYPE_NORMAL = 1u,
		TYPE_FIXED  = 2u,
		TYPE_UNDO   = 3u,
		TYPE_DIFF   = 4u,
	};
	uint32_t type;
	enum {
		FLAG_NONE        = 0x0000u,
		FLAG_SPLIT2G     = 0x0001u,
		FLAG_ZERO_EXPAND = 0x0100u,
	};
	uint32_t flags;
	enum { COMMENT_SIZE = 256u, };
	char comment[COMMENT_SIZE]; /* UTF-8 */
	uint32_t blocks_offset; /* block table */
	uint32_t data_offset;
	Disk_geometry legacy_geometry;
	uint32_t bios_hdd_trans_mode; /* unused */
	uint64_t disk_size;
	enum {
		SECTOR_SIZE = 512,
		BLOCK_SIZE = 1u<<20,
	};
	uint32_t block_size; /* 1MiB */
	uint32_t block_size_extra;
	uint32_t blocks; /* number of blocks */
	uint32_t allocated_blocks;
	Random_uuid image_uuid;
	Random_uuid modify_uuid;
	Random_uuid prev_uuid;
	Random_uuid prev_modify_uuid;
	Disk_geometry logical_geometry;
} __attribute__((packed));

#endif /* _VDI_TYPES_H_ */
