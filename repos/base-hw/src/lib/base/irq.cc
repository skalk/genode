#include <base/internal/capability_space.h>
#include <kernel/interface.h>
#include <irq_session/connection.h>


void Genode::Irq_session_client::ack_irq()
{
	if (_sigh.valid()) { Kernel::ack_irq(Capability_space::capid(_sigh)); }
}


void Genode::Irq_session_client::sigh(Signal_context_capability sigh)
{
	call<Rpc_sigh>(sigh);
	_sigh = sigh;
}
