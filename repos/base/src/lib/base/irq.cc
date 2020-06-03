
#include <irq_session/client.h>

void Genode::Irq_session_client::ack_irq() {
	call<Rpc_ack_irq>(); }


void Genode::Irq_session_client::sigh(Signal_context_capability sigh) {
	call<Rpc_sigh>(sigh); }

