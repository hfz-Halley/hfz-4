
#include <EthernetHandler.h>
#include <string>

namespace Firefinch
{

namespace Net
{

std::string EthernetHandler::serviceName = "framework.system.networkmanager.ethernet";

EthernetHandler::EthernetHandler(DBus::Connection::pointer conn) : DBus::ObjectProxy(conn, "Firefinch.Ethernet.Service", "/Firefinch/Net/NetworkManager")
{

  m_open = this->create_method<bool, std::string>("EthernetHandler.open", "open");
  m_isOpen = this->create_method<bool, std::string>("EthernetHandler.isOpen", "isOpen");
  m_reconfigure = this->create_method<bool, std::string>("EthernetHandler.reconfigure", "reconfigure");
  m_setIPv4Address = this->create_method<bool, std::string, bool, std::string, std::string, std::string>("EthernetHandler.setIPv4Address", "setIPv4Address");
  m_getIPv4Address = this->create_method<std::string, std::string>("EthernetHandler.getIPv4Address", "getIPv4Address");
  m_setDNSAddress = this->create_method<bool, std::string, std::string, std::string>("EthernetHandler.setDNSAddress", "setDNSAddress");
  m_getDNSAddress = this->create_method<std::string, std::string>("EthernetHandler.getDNSAddress", "getDNSAddress");
  m_getConnectDetailed = this->create_method<std::string, std::string>("EthernetHandler.getConnectDetailed", "getConnectDetailed");
  m_close = this->create_method<bool, std::string>("EthernetHandler.close", "close");
}

bool EthernetHandler::isOpen(std::string name)
{
  return (*m_isOpen)(name);
}

bool EthernetHandler::open(std::string name)
{
  return (*m_open)(name);
}

bool EthernetHandler::reconfigure(std::string name)
{
  return (*m_reconfigure)(name);
}

bool EthernetHandler::setIPv4Address(std::string name, bool autoDhcp, std::string address, std::string netmask, std::string gateway)
{
  return (*m_setIPv4Address)(name, autoDhcp, address, netmask, gateway);
}

std::string EthernetHandler::getIPv4Address(std::string name)
{
  return (*m_getIPv4Address)(name);
}

bool EthernetHandler::setDNSAddress(std::string name, std::string primary, std::string secondary)
{
  return (*m_setDNSAddress)(name, primary, secondary);
}

std::string EthernetHandler::getDNSAddress(std::string name)
{
  return (*m_getDNSAddress)(name);
}

std::string EthernetHandler::getConnectDetailed(std::string name)
{
  return (*m_getConnectDetailed)(name);
}

bool EthernetHandler::close(std::string name)
{
  return (*m_close)(name);
}

} // namespace Net

} // namespace Firefinch
