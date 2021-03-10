#include <i2c_session/connection.h>
#include <lx_kit/env.h>
#include <lx_emul/i2c.h>

using Transaction = I2c::Session::Transaction;
using Message     = I2c::Session::Message;

static I2c::Connection & i2c()
{
	static I2c::Connection i2c(Lx_kit::env().env);
	return i2c;
}


extern "C" unsigned char lx_emul_i2c_read_byte(unsigned char addr)
{
	Transaction t(Message(Message::WRITE, addr), Message(Message::READ, 0));
	i2c().transmit(t);
	return t.value(1).value(0);
}


extern "C" void lx_emul_i2c_write_byte(unsigned char addr, unsigned char byte)
{
	Transaction t(Message(Message::WRITE, addr, byte));
	i2c().transmit(t);
}
