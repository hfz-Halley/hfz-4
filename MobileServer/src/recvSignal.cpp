
#include <dbus-cxx.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>

class recvSignal : public DBus::ObjectProxy
{
protected:
  recvSignal(DBus::Connection::pointer conn) : DBus::ObjectProxy(conn, "dbuscxx.mobile.connect.name", "/dbuscxx/mobileSignal/signal")
  {

    recv_method = this->create_method<bool, int, std::string>("mobile.signal", "recv");
    m_signal = this->create_signal<void, int, std::string>("mobile.signal", "emitSignal_fun");
  }

public:
  typedef DBusCxxPointer<recvSignal> pointer;

  static pointer create(DBus::Connection::pointer conn)
  {
    return pointer(new recvSignal(conn));
  }

  bool recv(int code, std::string str)
  {
    return (*recv_method)(code, str);
  }

  DBus::signal_proxy<void, int, std::string> &getsignal()
  {
    return *m_signal;
  }

protected:
  DBus::MethodProxy<bool, int, std::string>::pointer recv_method;
  DBus::signal_proxy<void, int, std::string>::pointer m_signal;
};

void print(int code, std::string str)
{
  printf("\n状态码:%02x\n", code);
  printf("信息:%s\n", str.c_str());
}

int tttbbb()
{

  DBus::init();
  DBus::Dispatcher::pointer dispatcher = DBus::Dispatcher::create();
  DBus::Connection::pointer connection = dispatcher->create_connection(DBus::BUS_SESSION);

  recvSignal::pointer obj = recvSignal::create(connection);

  obj->getsignal().connect(sigc::ptr_fun(print));

  std::cout << "Running" << std::flush;

  while (1)
  {
    std::cout << "." << std::flush;
    sleep(20);
  }

  return 0;
}
