/*
 * \brief  Mini C abort()
 * \author Christian Helmuth
 * \date   2008-07-24
 */

/*
 * Copyright (C) 2008-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/log.h>
#include <base/sleep.h>

extern "C" void abort(void)
{
	Genode::warning("abort called");
	Genode::sleep_forever();
}
