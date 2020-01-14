/*
 * \brief  XML configuration for wired NIC driver
 * \author Norman Feske
 * \date   2018-05-02
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <runtime.h>

void Sculpt::gen_nic_drv_start_content(Xml_generator &xml)
{
	gen_common_start_content(xml, "nic_drv", Cap_quota{300}, Ram_quota{20*1024*1024});

	gen_named_node(xml, "resource", "CPU", [&] () {
		xml.attribute("quantum", "10"); });

	xml.node("binary", [&] () { xml.attribute("name", "fec_nic_drv"); });

	gen_provides<Nic::Session>(xml);

	xml.node("config", [&] () {
		xml.node("card", [&] () {
			xml.attribute("type", "fsl,imx6sx-fec");
			xml.attribute("mii", "rgmii");
			xml.attribute("irq", "152");
			xml.attribute("mmio", "0x30be0000");
		});
	});

	xml.node("route", [&] () {
		gen_parent_rom_route(xml, "fec_nic_drv");
		gen_parent_rom_route(xml, "ld.lib.so");
		gen_parent_route<Cpu_session>    (xml);
		gen_parent_route<Pd_session>     (xml);
		gen_parent_route<Rm_session>     (xml);
		gen_parent_route<Log_session>    (xml);
		gen_parent_route<Io_mem_session> (xml);
		gen_parent_route<Irq_session>    (xml);
		gen_parent_route<Timer::Session> (xml);
	});

}
