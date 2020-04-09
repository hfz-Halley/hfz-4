
#ifndef NET_4G_H
#define NET_4G_H

#include <dbus-cxx.h>
#include <string.h>

namespace Firefinch
{

namespace Net
{

class net4Gcaller : public DBus::ObjectProxy
{

protected:
	net4Gcaller(DBus::Connection::pointer conn) : DBus::ObjectProxy(conn, "Firefinch.Network.Service.Mobile", "/firefinch/Net/mobileServer")
	{

		m_sendMessage = this->create_method<bool, std::string, std::string>("MobileHandler.sendMessage", "sendMessage");
		m_getMobileInfo = this->create_method<std::string>("MobileHandler.getMobileInfo", "getMobileInfo");
		m_disconnect = this->create_method<bool>("MobileHandler.disconnect", "disconnect");
		m_connect = this->create_method<bool>("MobileHandler.connect", "connect");
	}

public:
	typedef DBusCxxPointer<net4Gcaller> pointer;
	static pointer create(DBus::Connection::pointer conn)
	{
		return pointer(new net4Gcaller(conn));
	}

	bool sendMessage(std::string phonenum, std::string MSG)
	{

		return (*m_sendMessage)(phonenum, MSG);
	}

	std::string getMobileInfo()
	{
		return (*m_getMobileInfo)();
	}
	bool connect() { return (*m_connect)(); }
	bool disconnect() { return (*m_disconnect)(); }

protected:
	DBus::MethodProxy<bool>::pointer m_disconnect;
	DBus::MethodProxy<bool>::pointer m_connect;
	DBus::MethodProxy<bool, std::string, std::string>::pointer m_sendMessage;
	DBus::MethodProxy<std::string>::pointer m_getMobileInfo;
};

} // namespace Net

} // namespace Firefinch

#endif
