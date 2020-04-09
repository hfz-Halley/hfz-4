#ifndef _Quectel_mobile_H
#define _Quectel_mobile_H

#include <Poco/Thread.h>
#include "GobiNetThread.h"
#include "QMIThread.h"
#include "mobileImpl.h"

namespace Firefinch
{

namespace Net
{

class Quectel_mobile : public mobileImpl
{
public:
	Quectel_mobile();
	virtual ~Quectel_mobile();
	virtual bool connect();
	virtual bool disconnect();
	virtual std::string getMobileInfo();
	virtual bool checkNetRegsterStatus();

protected:
	bool checkQmidevice();
	bool getconfiguration();
	int queryConnectState();
	int getSIMStatus();
	int getNetworkMode();
	void recvNotify(const void *pSender, int &arg);
	void startGobithread() { _Gobithread.start(); }
	void stopGobithread() { _Gobithread.stop(); }

private:
	PROFILE_T *profile;
	CHAR pDataCapStr[20] = "UNKNOW";
	std::string DataCap;
	std::string ethDetailed;
	GobiNethread _Gobithread;
	Poco::Logger &_logger;
	bool connectFlag;
	bool isReady;
	SIM_Status SIMStatus;
	UCHAR PSAttachedState;
	UCHAR IPv4ConnectionStatus; //unknow state
	UCHAR IPV6ConnectionStatus; //unknow state
};

} // namespace Net

} // namespace Firefinch

#endif

//POCO_SERVER_MAIN(Quectel_mobile)
