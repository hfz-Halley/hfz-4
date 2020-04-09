#ifndef MOBILESIGAL_H
#define MOBILESIGAL_H

#include <dbus-cxx.h>
#include <iostream>

namespace Firefinch
{

namespace Net
{

class mobileSignal : public DBus::Object
{
protected:
	mobileSignal() : DBus::Object("/dbuscxx/mobileSignal/signal")
	{

		DBus::init();
		int ret;
		dispatcher = DBus::Dispatcher::create();
		conn = dispatcher->create_connection(DBus::BUS_SESSION);
		// request a name on the bus
		ret = conn->request_name("dbuscxx.mobile.connect.name", DBUS_NAME_FLAG_REPLACE_EXISTING);
		if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)

			this->create_method<void, int, std::string>("mobile.signal", "emitSignal", sigc::mem_fun(*this, &mobileSignal::emitSignal));
		m_signal = this->create_signal<void, int, std::string>("mobile.signal", "emitSignal_fun");
	}

	DBus::signal<void, int, std::string>::pointer m_signal;
	DBus::Connection::pointer conn;
	DBus::Dispatcher::pointer dispatcher;

public:
	typedef DBusCxxPointer<mobileSignal> pointer;

	static pointer create()
	{

		return pointer(new mobileSignal());
	}

	void emitSignal(int code, std::string str)
	{
		// std::cout<<"emit ....."<<str<<std::endl;
		m_signal->emit(code, str);
	}

	DBus::signal<void, int, std::string> &getsignal()
	{

		return *m_signal;
	}
	DBus::Connection::pointer getconn()
	{

		return conn;
	}
};

} // namespace Net

} // namespace Firefinch

#endif
