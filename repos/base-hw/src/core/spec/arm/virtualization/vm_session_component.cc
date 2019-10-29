/*
 * \brief  VM session component for 'base-hw'
 * \author Stefan Kalkowski
 * \date   2015-02-17
 */

/*
 * Copyright (C) 2015-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <util/construct_at.h>

/* core includes */
#include <kernel/core_interface.h>
#include <spec/arm/virtualization/vm_session_component.h>
#include <platform.h>
#include <cpu_thread_component.h>
#include <core_env.h>

using namespace Genode;

static Core_mem_allocator & cma() {
	return static_cast<Core_mem_allocator&>(platform().core_mem_alloc()); }


void Vm_session_component::_exception_handler(Signal_context_capability handler,
                                              Vcpu_id id)
{
	if (!_valid_id(id)) {
		Genode::warning("invalid vcpu id ", id.id);
		return;
	}

	Vcpu & vcpu = _vcpus[id.id];
	if (vcpu.kobj.constructed()) {
		Genode::warning("Cannot register vcpu handler twice");
		return;
	}

	unsigned const cpu = vcpu.location.valid() ? vcpu.location.xpos() : 0;

	if (!vcpu.kobj.create(cpu, vcpu.ds_addr, Capability_space::capid(handler),
	                      cma().phys_addr(&_table)))
		Genode::warning("Cannot instantiate vm kernel object, ",
		                "invalid signal context?");
}


void Vm_session_component::_attach(addr_t phys_addr, addr_t vm_addr, size_t size)
{
	using namespace Hw;

	Page_flags pflags { RW, NO_EXEC, USER, NO_GLOBAL, RAM, CACHED };

	try {
		_table.insert_translation(vm_addr, phys_addr, size, pflags,
		                          _table_array.alloc());
		return;
	} catch(Hw::Out_of_tables &) {
		Genode::error("Translation table needs to much RAM");
	} catch(...) {
		Genode::error("Invalid mapping ", Genode::Hex(phys_addr), " -> ",
		              Genode::Hex(vm_addr), " (", size, ")");
	}
}


void Vm_session_component::_attach_vm_memory(Dataspace_component &dsc,
                                             addr_t const vm_addr,
                                             Attach_attr const attribute)
{
	_attach(dsc.phys_addr() + attribute.offset, vm_addr, attribute.size);
}


void Vm_session_component::attach_pic(addr_t vm_addr)
{
	_attach(Board::Cpu_mmio::IRQ_CONTROLLER_VT_CPU_BASE, vm_addr,
	        Board::Cpu_mmio::IRQ_CONTROLLER_VT_CPU_SIZE);
}


void Vm_session_component::_detach_vm_memory(addr_t vm_addr, size_t size)
{
	_table.remove_translation(vm_addr, size, _table_array.alloc());
}


void * Vm_session_component::_alloc_table()
{
	void * table;
	/* get some aligned space for the translation table */
	if (!cma().alloc_aligned(sizeof(Table), (void**)&table,
	                         Table::ALIGNM_LOG2).ok()) {
		error("failed to allocate kernel object");
		throw Insufficient_ram_quota();
	}
	return table;
}


Vm_session::Vcpu_id Vm_session_component::_create_vcpu(Thread_capability tcap)
{
	using namespace Genode;

	if (_id_alloc == VCPU_MAX) return Vcpu_id{Vcpu_id::INVALID};

	Affinity::Location vcpu_location;
	auto lambda = [&] (Cpu_thread_component *ptr) {
		if (!ptr) return;
		vcpu_location = ptr->platform_thread().affinity();
	};
	_ep.apply(tcap, lambda);

	Vcpu & vcpu = _vcpus[_id_alloc];
	vcpu.ds_cap = _constrained_md_ram_alloc.alloc(_ds_size(),
	                                              Cache_attribute::UNCACHED);
	try {
		vcpu.ds_addr = _region_map.attach(vcpu.ds_cap);
	} catch (...) {
		_constrained_md_ram_alloc.free(vcpu.ds_cap);
		throw;
	}

	vcpu.location = vcpu_location;
	return Vcpu_id { _id_alloc++ };
}


Genode::Dataspace_capability
Vm_session_component::_cpu_state(Vm_session::Vcpu_id id)
{
	return (_valid_id(id)) ? _vcpus[id.id].ds_cap
	                       : Genode::Ram_dataspace_capability();
}


Vm_session_component::Vm_session_component(Rpc_entrypoint &ds_ep,
                                           Resources resources,
                                           Label const &,
                                           Diag,
                                           Ram_allocator &ram_alloc,
                                           Region_map &region_map,
                                           unsigned,
                                           Trace::Source_registry &)
: Ram_quota_guard(resources.ram_quota),
  Cap_quota_guard(resources.cap_quota),
  _ep(ds_ep),
  _constrained_md_ram_alloc(ram_alloc, _ram_quota_guard(), _cap_quota_guard()),
  _sliced_heap(_constrained_md_ram_alloc, region_map),
  _region_map(region_map),
  _table(*construct_at<Table>(_alloc_table())),
  _table_array(*(new (cma()) Array([this] (void * virt) {
	return (addr_t)cma().phys_addr(virt);})))
{
	/* configure managed VM area */
	_map.add_range(0, 0UL - 0x1000);
	_map.add_range(0UL - 0x1000, 0x1000);
}


Vm_session_component::~Vm_session_component()
{
	/* detach all regions */
	while (true) {
		addr_t out_addr = 0;

		if (!_map.any_block_addr(&out_addr))
			break;

		detach(out_addr);
	}

	/* free region in allocator */
	for (unsigned i = 0; i < _id_alloc; i++) {
		Vcpu & vcpu = _vcpus[i];
		if (vcpu.ds_cap.valid()) {
			_region_map.detach(vcpu.ds_addr);
			_constrained_md_ram_alloc.free(vcpu.ds_cap);
		}
	}

	/* free guest-to-host page tables */
	destroy(platform().core_mem_alloc(), &_table);
	destroy(platform().core_mem_alloc(), &_table_array);
}
