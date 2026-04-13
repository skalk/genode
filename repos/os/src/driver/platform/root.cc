/*
 * \brief  Platform driver - root component
 * \author Stefan Kalkowski
 * \date   2020-04-13
 */

/*
 * Copyright (C) 2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <root.h>

using Version = Driver::Session_component::Policy_version;

void Driver::Root::update_policy()
{
	_pds.for_each([&] (auto &pd) {
		with_matching_policy(pd.label(), _config.node(),
			[&] (auto const &policy) {
				pd.update_policy(policy.attribute_value("info", false),
				                 policy.attribute_value("version", Version()));
			},
			[&] {
				error("No matching policy for '", pd.label().string(),
				      "' anymore, will close all related sessions!");
				pd.close_all_sessions(*this);
				_pd_alloc.destroy(pd);
			});
	});
}


Driver::Root::Create_result Driver::Root::_create_session(const char *args)
{
	enum {
		PD_RAM_OVERHEAD = sizeof(Pd) + 4096,
		PD_CAP_OVERHEAD = 2
	};

	Session_label      label     = label_from_args(args);
	Session::Resources resources = session_resources_from_args(args);

	if (resources.cap_quota.value < PD_CAP_OVERHEAD)
		return Create_error::OUT_OF_CAPS;

	if (resources.ram_quota.value < PD_RAM_OVERHEAD)
		return Create_error::OUT_OF_RAM;

	resources.cap_quota.value -= PD_CAP_OVERHEAD;
	resources.ram_quota.value -= PD_RAM_OVERHEAD;

	auto no_pd_found = [&] (Node const &policy) -> Create_result {

		bool    info    = policy.attribute_value("info", false);
		Version version = policy.attribute_value("version", Version());

		return _pd_alloc.create(_env, _config, _devices, _pds, label,
		                        info, version).template convert<Create_result>(
			[&] (auto &a) {
				a.deallocate = false;
				return _alloc_obj(_env, a.obj, _devices, resources);
			},
			/*
			 * The platform driver needs to pay for PD objects  out of
			 * its own resources, because they are potentially shared in
			 * between different device and dma sessions, so in case
			 * of resource shortage, it just fails!
			 */
			[&] (auto) {
				if (_warn_once) {
				error("Platform driver out of resources, cannot create PD");
				_warn_once = false;
				}
				return Create_error::DENIED;
			});
	};

	return with_matching_policy(label, _config.node(),
	        [&] (Node const &policy) -> Create_result {
	                return _pds.with_element(label,
	                        [&] (Pd &pd) {
	                                return _alloc_obj(_env, pd, _devices, resources);
	                        },
	                        [&] { return no_pd_found(policy); });
		},
		[&] () -> Create_result {
			error("Invalid session request, no matching policy for ",
			      "'", label_from_args(args), "'");
			return Create_error::DENIED;
		});
}


void Driver::Root::_upgrade_session(Session_component &sc, const char * args)
{
	sc.upgrade(ram_quota_from_args(args));
	sc.upgrade(cap_quota_from_args(args));
}


void Driver::Root::_destroy_session(Session_component &sc)
{
	Pd &pd = sc._pd;
	Root_component<Session_component>::_destroy_session(sc);

	if (pd.empty()) _pd_alloc.destroy(pd);
}


Driver::Root::Root(Env                          &env,
                   Sliced_heap                  &sliced_heap,
                   Heap                         &heap,
                   Attached_rom_dataspace const &config,
                   Device_model                 &devices)
:
	Root_component<Session_component>(env.ep(), sliced_heap),
	_env(env), _config(config), _devices(devices), _pd_alloc(heap)
{ }
