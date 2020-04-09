#ifndef _SIM7600_H
#define _SIM7600_H

#include <Poco/Thread.h>
#include "Poco/AutoPtr.h"
#include "Poco/Logger.h"
#include "mobileImpl.h"
#include "AtCommand.h"

namespace Firefinch
{

namespace Net
{

class Sim7600_mobile : public mobileImpl
{

public:
	Sim7600_mobile(Poco::Serial::ATcommand *cmd);
	virtual ~Sim7600_mobile();
	virtual bool connect();
	virtual bool disconnect();
	virtual std::string getMobileInfo();
	virtual bool checkNetRegsterStatus();

protected:
private:
	int SIMStatus;
	int NetAttachStatus;
	int NetworkMode;
	Poco::Logger &_logger;
	Poco::Serial::ATcommand *AtCmd;
};

} // namespace Net

} // namespace Firefinch

#endif
