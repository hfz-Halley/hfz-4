#ifndef _MOBILEMANAGER_H
#define _MOBILEMANAGER_H

#include "Poco/Logger.h"
#include <dbus-cxx.h>
#include "Sim7600_mobile.h"
#include "Quectel_mobile.h"
#include "iostream"
#include "AtCommand.h"
#include "EthernetHandler.h"
#include "mobileSignal.h"
#include "Poco/Timer.h"
#include <Poco/Mutex.h>
#include "Poco/Event.h"

namespace Firefinch
{

namespace Net
{
class mobileManager : public DBus::Object
{

public:
	typedef DBusCxxPointer<mobileManager> pointer;
	static pointer create() { return pointer(new mobileManager()); }
	mobileManager();
	~mobileManager();
	bool initDevice();
	bool checkpaths(std::string path);
	bool sendMessage(std::string phoneNumber, std::string msg);
	void initConfigFile();
	void getSignalStrength(Poco::Timer &t);

	bool on_connect();
	bool on_disconnect();
	std::string on_getMobileInfo();
	bool on_checkNetRegsterStatus();

protected:
private:
	std::string eth2Path;
	std::string wwan0Path;
	std::string devName;
	std::string ethDetailed;
	Poco::Serial::ATcommand *command;
	mobileImpl *mobilePtr;
	DBus::Dispatcher::pointer ethdispatcher;
	DBus::Connection::pointer ethconn;
	EthernetHandler::pointer ethHandler;
	mobileSignal::pointer signalhandle;
	Poco::Logger &_logger;
	Poco::Timer signalTimer;
	Poco::TimerCallback<mobileManager> SignalCallback;
	Poco::Event _waitRunEvent;
	bool signalFlag;
	int SignalStrength;
};

} // namespace Net

} // namespace Firefinch

#endif
