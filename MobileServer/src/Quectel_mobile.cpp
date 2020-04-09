#include "Poco/Delegate.h"
#include <Poco/BasicEvent.h>
#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Quectel_mobile.h"
#include "Gobifunction.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Query.h"
#include "Poco/JSON/Parser.h"

namespace Firefinch
{

namespace Net
{

#define TAG "Quectel_mobile"

Quectel_mobile::Quectel_mobile() : _logger(Poco::Logger::get(TAG))
{
	profile = new PROFILE_T();
	connectFlag = false;
	profile->qmichannel = NULL;
	profile->usbnet_adapter = NULL;
	profile->apn = "3gnet";
	profile->user = profile->password = profile->pincode = NULL;
	profile->auth = 0;
	profile->pdp = CONFIG_DEFAULT_PDP;
	profile->IsDualIPSupported = 0;
	profile->curIpFamily = IpFamilyV4;

	IPv4ConnectionStatus = 0xff; //unknow state
	IPV6ConnectionStatus = 0xff; //unknow state
	isReady = false;
	_Gobithread.theEvent += Poco::delegate(this, &Quectel_mobile::recvNotify);
	checkQmidevice();
	startGobithread();
}

Quectel_mobile::~Quectel_mobile()
{
	stopGobithread();
	if (profile)
		delete profile;
}

std::string Quectel_mobile::getMobileInfo()
{
	Poco::JSON::Object jsnObj;
	std::stringstream jsnString;
	jsnObj.set("SIMStatus",getSIMStatus());
	//jsnObj.set( "ConnectStatus", queryConnectState());
	jsnObj.set("NetStatus", PSAttachedState ? 1 : 0);
	jsnObj.set("NetMode", getNetworkMode());
	jsnObj.stringify(jsnString, 3);
	return jsnString.str();
}

bool Quectel_mobile::checkQmidevice()
{

	if (NULL == profile->qmichannel)
	{
		if (qmidevice_detect(&profile->qmichannel, &profile->usbnet_adapter))
		{
			if (access(profile->qmichannel, R_OK | W_OK))
			{
				_logger.error("Fail to access  errno");
				return false;
			}
			if (!kill_brothers(profile))
				return true;
		}
	}
	return false;
}

bool Quectel_mobile::checkNetRegsterStatus()
{
	if (getSIMStatus() == SIM_READY && isReady && PSAttachedState)
		return true;
	return false;
}

int Quectel_mobile::getNetworkMode()
{
	if (pDataCapStr == std::string("LTE")) //4G
		return 1;
	return 0;
}

bool Quectel_mobile::connect()
{
	requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
	if (QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus)
	{
		if (requestSetupDataCall(profile, IpFamilyV4) == 0)
		{
			connectFlag = true;
			return true;
		}
	}
	connectFlag = false;
	return false;
}

bool Quectel_mobile::disconnect()
{
	requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
	if (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus)
	{
		if (requestDeactivateDefaultPDP(profile, IpFamilyV4) == 0)
		{
			connectFlag = false;
			return true;
		}
	}
	connectFlag = true;
	return false;
}
int Quectel_mobile::queryConnectState()
{
	if (!requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4) && (QWDS_PKT_DATA_CONNECTED == IPv4ConnectionStatus) && PSAttachedState == 1)
	{
		return QWDS_PKT_DATA_CONNECTED;
	}
	return QWDS_PKT_DATA_DISCONNECTED;
}

int Quectel_mobile::getSIMStatus()
{
	requestGetSIMStatus(&SIMStatus);
	if (SIMStatus == 0x01 || SIMStatus == 0x02)
		return SIMStatus;
	else
		return -1;
}

bool Quectel_mobile::getconfiguration()
{
	requestBaseBandVersion(NULL);
	requestSetEthMode(profile);
	requestGetSIMStatus(&SIMStatus);
	if ((SIMStatus == SIM_PIN) && profile->pincode)
	{
		requestEnterSimPin(profile->pincode);
	}
	requestGetProfile(profile);
	requestRegistrationState(&PSAttachedState, pDataCapStr);
	return true;
}

void Quectel_mobile::recvNotify(const void *pSender, int &triger_event)
{
	if (triger_event == RIL_INDICATE_DEVICE_CONNECTED)
	{
		getconfiguration();
		isReady = true;
		_logger.information("network registration is ready");
	}
	else if (triger_event == RIL_INDICATE_DEVICE_DISCONNECTED)
	{
		_logger.information(" triger_event== RIL_INDICATE_DEVICE_DISCONNECTED   please reboot device");
		exit(-1);
	}
	else if (triger_event == RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED)
	{
		requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
		if (PSAttachedState == 1 && QWDS_PKT_DATA_DISCONNECTED == IPv4ConnectionStatus)
		{
			if (connectFlag == true)
			{
				_logger.information(" triger_event==NETWORK_STATE_CHANGED");
			}
		}
	}
	else if (triger_event == RIL_UNSOL_DATA_CALL_LIST_CHANGED)
	{

		requestQueryDataCall(&IPv4ConnectionStatus, IpFamilyV4);
		requestRegistrationState(&PSAttachedState, pDataCapStr);
		if (connectFlag == true && QWDS_PKT_DATA_CONNECTED != IPv4ConnectionStatus)
		{
			_logger.information("DATA_CALL_LIST_CHANGED--reconnection");
			connect();
		}
	}
	else
	{
		if (triger_event)
			_logger.information("no bobdy care this triger_event");
	}
}

} // namespace Net

} // namespace Firefinch
