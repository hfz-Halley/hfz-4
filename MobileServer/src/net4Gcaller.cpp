#include <unistd.h>
#include <iostream>
#include "net4Gcaller.h"
#include <dbus-cxx.h>
#include <string.h>
#include <stdio.h>
using namespace Firefinch::Net;

//caller_test

int test(int argc, char *argv[])
{
	DBus::init();

	DBus::Dispatcher::pointer dispatcher = DBus::Dispatcher::create();

	DBus::Connection::pointer connection = dispatcher->create_connection(DBus::BUS_SESSION);

	net4Gcaller::pointer net4Gcaller = net4Gcaller::create(connection);

#if 1

	if (argc > 2)
		return -1;

	if (!strcasecmp(argv[1], "con"))
	{
		printf("connect ret ==%d\n", net4Gcaller->connect());
	}
	if (!strcasecmp(argv[1], "dis"))
	{
		printf("disconnect ret ==%d\n", net4Gcaller->disconnect());
	}
	else if (!strcasecmp(argv[1], "get"))
	{
		printf("getMobileInfo ==%s\n", net4Gcaller->getMobileInfo().c_str());
	}

	else if (!strcasecmp(argv[1], "send"))
	{
		std::string Msg = "每个汉字以及符号都有固定 字以及符号都有固定的Unicode代码。用相应的转换器转换";

		printf("send ==%d   ,msg size =%d  \n", net4Gcaller->sendMessage("13001286091", Msg),Msg.size() ); //strlen(Msg.c_str())
	}
#endif

	return 0;
}
