#include <trace/policy.h>
#include <trace/timestamp.h>
#include <util/string.h>

using namespace Genode;

size_t max_event_size()
{
	return 0;
}

static const size_t event(char *dst, char const *rpc_name, char const type)
{
	constexpr char string_len = 15;

	size_t rpc_name_len = strlen(rpc_name);

	Genode::Trace::Timestamp const time = Genode::Trace::timestamp();

	memcpy(dst, &type, 1);
	memcpy(dst + 1, &time, sizeof(time));
	memcpy(dst + 1 + sizeof(time), rpc_name, string_len);

	if (rpc_name_len < string_len)
		dst[1 + sizeof(time) + rpc_name_len] = 0;
	else
		dst[1 + sizeof(time) + string_len - 1] = 0;

	return 1 + sizeof(time) + string_len;
}

size_t rpc_call(char *dst, char const * rpc_name, Msgbuf_base const &)
{
	return event(dst, rpc_name, 'C');
}

size_t rpc_returned(char *dst, char const *rpc_name, Msgbuf_base const &)
{
	return event(dst, rpc_name, 'R');
}

size_t rpc_dispatch(char *dst, char const *rpc_name)
{
	return event(dst, rpc_name, 'D');
}

size_t rpc_reply(char *dst, char const *rpc_name)
{
	return event(dst, rpc_name, 'Y');
}

size_t signal_submit(char *dst, unsigned const)
{
	return 0;
}

size_t signal_receive(char *dst, Signal_context const &, unsigned)
{
	return 0;
}

