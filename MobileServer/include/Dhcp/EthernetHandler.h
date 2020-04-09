
#ifndef FRAMEWORK_SYSTEM_NETWORKMANAGER_EthernetHandler_H
#define FRAMEWORK_SYSTEM_NETWORKMANAGER_EthernetHandler_H

#include <dbus-cxx.h>
#include <Poco/RefCountedObject.h>
#include <Poco/AutoPtr.h>

namespace Firefinch{

namespace Net{

class EthernetHandler: public Poco::RefCountedObject, DBus::ObjectProxy
{
  public:
    EthernetHandler(DBus::Connection::pointer conn);
    typedef Poco::AutoPtr<EthernetHandler> Ptr;

    typedef DBusCxxPointer<EthernetHandler> pointer;
	
	static pointer create(DBus::Connection::pointer conn) {
		return pointer(new EthernetHandler(conn));
    }
    static std::string serviceName;

    bool open(std::string name);
    bool isOpen(std::string name);
    bool reconfigure(std::string name);
    bool setIPv4Address(std::string name, bool autoDhcp, std::string address, std::string netmask, std::string gateway);
    std::string getIPv4Address(std::string name);
    bool setExRouteAddress(bool, std::string addressList); ///set extern route address
    std::string getExRouteAddress(std::string name);
    bool setDNSAddress(std::string name, std::string primary, std::string secondary);
    std::string getDNSAddress(std::string name);
    std::string getConnectDetailed(std::string name);
    bool close(std::string name);

  protected:
    DBus::MethodProxy<bool, std::string>::pointer m_isOpen;
    DBus::MethodProxy<bool, std::string>::pointer m_open;
    DBus::MethodProxy<bool, std::string>::pointer m_reconfigure;
    DBus::MethodProxy<bool, std::string, bool, std::string, std::string, std::string>::pointer m_setIPv4Address;
    DBus::MethodProxy<std::string, std::string>::pointer m_getIPv4Address;
    DBus::MethodProxy<bool, std::string, std::string, std::string>::pointer m_setDNSAddress;
    DBus::MethodProxy<std::string, std::string>::pointer m_getDNSAddress;
    DBus::MethodProxy<std::string, std::string>::pointer m_getConnectDetailed;
    DBus::MethodProxy<bool, std::string>::pointer m_close;

  private:

};



}

}






#endif
