#include "mobileManager.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Util/JSONConfiguration.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Query.h"
#include <unistd.h>

using namespace Poco::Serial;

namespace Firefinch
{

namespace Net
{

#define CONFIGURE_FILE_PATH "/usr/firefinch/mobile_network/etc/configure.json"

mobileManager::mobileManager() : DBus::Object("/firefinch/Net/mobileServer"),
								 _logger(Poco::Logger::get("mobileManager")),
								 signalTimer(0, 10000),
								 SignalCallback(*this, &mobileManager::getSignalStrength),
								 _waitRunEvent(true)
{
	eth2Path = "/sys/class/net/eth2";
	wwan0Path = "/sys/class/net/wwan0";
	signalFlag = false;

	ethdispatcher = DBus::Dispatcher::create();
	ethconn = ethdispatcher->create_connection(DBus::BUS_SESSION);
	ethHandler = EthernetHandler::create(ethconn);
	signalhandle = mobileSignal::create();
	signalhandle->getconn()->register_object(signalhandle);
	SignalStrength = 0;

	this->create_method<bool>("connect", sigc::mem_fun(this, &mobileManager::on_connect));
	this->create_method<bool>("disconnect", sigc::mem_fun(this, &mobileManager::on_disconnect));
	this->create_method<bool, std::string, std::string>("sendMessage", sigc::mem_fun(this, &mobileManager::sendMessage));
	this->create_method<std::string>("getMobileInfo", sigc::mem_fun(this, &mobileManager::on_getMobileInfo));
}
mobileManager::~mobileManager()
{
	mobilePtr->disconnect();
	signalTimer.stop();
	if (command)
		delete command;
	if (mobilePtr)
		delete mobilePtr;
}

bool mobileManager::initDevice()
{
	if (checkpaths(eth2Path))
	{
		command = new ATcommand();
		mobilePtr = new Quectel_mobile();
		devName = "eth2";
		for (int i = 0; i < 3; i++)
		{
			sleep(1);
			if (mobilePtr->checkNetRegsterStatus())
			{
				initConfigFile();
				signalTimer.start(SignalCallback);
				return true;
			}
		}
		return false;
	}
	else if (checkpaths(wwan0Path))
	{
		command = new ATcommand();
		mobilePtr = new Sim7600_mobile(command);
		devName = "wwan0";
		if ( command->set_cnsmod() &&  mobilePtr->checkNetRegsterStatus() )//command->setKeepalive() && command->set_cnsmod()  &&
		{
			_logger.information("Parser json file and init ");
			initConfigFile();
			signalTimer.start(SignalCallback);
		}
		return true;
	}
	else
		_logger.error("The device cannot be found");


	return false;
}





bool mobileManager::checkpaths(std::string path)
{
	Poco::File paths = Poco::File(Poco::Path(path));
	if (paths.exists())
	{
		_logger.information("%s is exists", path);
		return true;
	}
	_logger.error("%s is not exists", path);
	return false;
}

void mobileManager::getSignalStrength(Poco::Timer &t)
{
	if (signalFlag)
	{
		if (command->GetRssidata())
		{	
			SignalStrength = command->GetCurSignalStrenght();
			signalhandle->emitSignal(SignalStrength, "signal");
			_logger.information("SignalStrenght = %d ",SignalStrength);
		}
	}
}

bool mobileManager::sendMessage(std::string phoneNumber, std::string msg)
{
	signalFlag = false;
	if (msg.size() < 120 && msg.size() > 0 && phoneNumber.size() == 11)
	{	
		_logger.information("sending...");
		std::string number(Poco::format("+86%s", phoneNumber));
		if (command->sendPduMessage(number, msg))
		{
			signalFlag = true;
			return true;
		}
	}
	signalFlag = true;
	_logger.error("Message sending failed");
	return false;
}

bool mobileManager::on_connect()
{
	signalFlag = false;
	if (mobilePtr->connect())
	{		
		signalFlag = true;
		if (ethHandler->open(devName))
			return ethHandler->reconfigure(devName);
	}
	signalFlag = true;
	_logger.error("on_Connect error");
	return false;
}
bool mobileManager::on_disconnect()
{
	signalFlag = false;
	if (mobilePtr->disconnect())
	{
		signalFlag = true;
		if (ethHandler->isOpen(devName))
		{
			return ethHandler->close(devName);
		}
		_logger.information(" eth2Handler close ");
		return true;
	}
	signalFlag = true;
	_logger.error("on_Disconnect error");
	return false;
}
std::string mobileManager::on_getMobileInfo()
{
	Poco::JSON::Object jsnObj;
	std::stringstream jsnString;
	Poco::JSON::Parser jsonParser;
	if(on_checkNetRegsterStatus())
	{
		ethDetailed = ethHandler->getConnectDetailed(devName);
		Poco::Dynamic::Var result = jsonParser.parse(ethDetailed);
		Poco::JSON::Object::Ptr requestObj = result.extract<Poco::JSON::Object::Ptr>();
		Poco::Dynamic::Var ipv4result = requestObj->get("ipv4");
		Poco::JSON::Object::Ptr ipv4obj = ipv4result.extract<Poco::JSON::Object::Ptr>();

		jsnObj.set("Address", ipv4obj->get("address"));
		jsnObj.set("Netmask", ipv4obj->get("netmask"));
		jsnObj.set("Gateway", ipv4obj->get("gateway"));
		jsnObj.set("DNS", requestObj->get("dns"));
		jsnObj.set("IsPlugin", requestObj->get("isPlugin"));
		jsnObj.set("MacAddress", requestObj->get("macAddress"));

		result = jsonParser.parse(mobilePtr->getMobileInfo());
		Poco::JSON::Object::Ptr simcard = result.extract<Poco::JSON::Object::Ptr>();
		jsnObj.set("SIMStatus", simcard->get("SIMStatus")); //sim卡状态 2：插入        1；未插入
		jsnObj.set("NetStatus", simcard->get("NetStatus")); //网络数据业务状态
		jsnObj.set("NetMode", simcard->get("NetMode"));		//4G 、非4G
		jsnObj.stringify(jsnString, 3);
		return jsnString.str();
	}
	return "error";

}

bool mobileManager::on_checkNetRegsterStatus()
{
	signalFlag = false;
	bool ret = mobilePtr->checkNetRegsterStatus();
	signalFlag = true;
	return ret;
}

void mobileManager::initConfigFile()
{
	try
	{
		Poco::AutoPtr<Poco::Util::JSONConfiguration> configFile = new Poco::Util::JSONConfiguration(CONFIGURE_FILE_PATH);
		if (!configFile)
		{
			throw Poco::NotFoundException("Configure file is miss!");
		}
		std::stringstream out;
		configFile->save(out);
		Poco::JSON::Parser jsonParse;
		Poco::Dynamic::Var result;
		result = jsonParse.parse(out.str());
		Poco::JSON::Object::Ptr _configureJson = result.extract<Poco::JSON::Object::Ptr>();
		if (_configureJson)
		{
			bool mConnect = _configureJson->getValue<bool>("connect");
			if (mConnect == true)
			{
				if (on_connect())
				{
					_logger.information("connect is successfully");
				}
				else
				{
					_logger.information("connect is failed");
				}
			}
		}
	}
	catch (Poco::JSON::JSONException &jsone)
	{
		// _mLogger.error("Parse:" + jsone.message());
	}
}

} // namespace Net

} // namespace Firefinch
