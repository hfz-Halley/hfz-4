#include "Sim7600_mobile.h"
#include <string.h>
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Query.h"

namespace Firefinch
{

namespace Net
{

Sim7600_mobile::Sim7600_mobile(Poco::Serial::ATcommand *cmd) : _logger(Poco::Logger::get("Sim7600_mobile"))
{
	AtCmd = cmd;
	SIMStatus = 0;
	NetworkMode = 0;
	NetAttachStatus = 0;
}
Sim7600_mobile::~Sim7600_mobile()
{
}
bool Sim7600_mobile::connect()
{
	char buf[64] = {0};
	_logger.information("connecting....");
	if (AtCmd->AT_Send((char *)"AT$QCRMCALL=1,1", buf, 64))
	{
		printf("buf == %s \n", buf);
		if (strstr(buf, "OK") || strstr(buf, "V4"))
		{
			return true;
		}
	}
	_logger.error("data connect failed! ");
	return false;
}

bool Sim7600_mobile::disconnect()
{
	char buf[64] = {0};
	_logger.information("disconnecting....");
	if (AtCmd->AT_Send((char *)"AT$QCRMCALL=0,1", buf, 64))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}
	_logger.error("data disconnect failed! ");
	return false;
}
std::string Sim7600_mobile::getMobileInfo()
{
	_logger.information("getMobileInfo");
	Poco::JSON::Object jsnObj;
	std::stringstream jsnString;
	jsnObj.set("SIMStatus", SIMStatus);
	jsnObj.set("NetStatus", NetAttachStatus); //数据业务状态
	jsnObj.set("NetMode", NetworkMode);		  // 4G  、非4G
	jsnObj.stringify(jsnString, 3);
	return jsnString.str();
}
bool Sim7600_mobile::checkNetRegsterStatus()
{
	SIMStatus = AtCmd->getSIMStatus();
	NetworkMode = AtCmd->getNetworkMode();
	NetAttachStatus = AtCmd->getNetAttachStatus();
	if (SIMStatus == 2 && NetAttachStatus == 1)
		return true;
	return false;
}

} // namespace Net

} // namespace Firefinch
