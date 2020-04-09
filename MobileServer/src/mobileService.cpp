#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/AbstractConfiguration.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/ErrorHandler.h"
#include "Poco/Environment.h"
#include "Poco/Format.h"
#include "Poco/File.h"
#include <cstring>
#include <iostream>
#include <memory>
#include "mobileManager.h"
#include "Poco/Thread.h"
#include "Poco/BasicEvent.h"

using Poco::BasicEvent;
using Poco::Util::AbstractConfiguration;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;
using namespace Firefinch::Net;

class mobileService : public ServerApplication
{
public:
	mobileService() : _errorHandler(*this),
					  _showHelp(false)
	{
		Poco::ErrorHandler::set(&_errorHandler);
	}

	~mobileService()
	{
	}

protected:
	class ErrorHandler : public Poco::ErrorHandler
	{
	public:
		ErrorHandler(mobileService &app) : _app(app)
		{
		}

		void exception(const Poco::Exception &exc)
		{
			// Don't log Poco::Net::ConnectionResetException and Poco::TimeoutException -
			// getting too many of them from the web server.
			if (std::strcmp(exc.name(), "Connection reset by peer") != 0 &&
				std::strcmp(exc.name(), "Timeout") != 0)
			{
				log(exc.displayText());
			}
		}

		void exception(const std::exception &exc)
		{
			log(exc.what());
		}

		void exception()
		{
			log("unknown exception");
		}

		void log(const std::string &message)
		{
			_app.logger().notice("A thread was terminated by an unhandled exception: " + message);
		}

	private:
		mobileService &_app;
	};

	void initialize(Application &self)
	{
		loadConfiguration(); // load default configuration files, if present

		ServerApplication::initialize(self);
	}

	void defineOptions(OptionSet &options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "Display help information on command line arguments.")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<mobileService>(this, &mobileService::handleHelp)));

		options.addOption(
			Option("config-file", "c", "Load configuration data from a file.")
				.required(false)
				.repeatable(true)
				.argument("file")
				.callback(OptionCallback<mobileService>(this, &mobileService::handleConfig)));
	}

	void handleHelp(const std::string &name, const std::string &value)
	{
		_showHelp = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void handleConfig(const std::string &name, const std::string &value)
	{
		loadConfiguration(value);
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("The rosefinch server application");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string> &args)
	{
		DBus::init();
		DBus::Dispatcher::pointer dispatcher = DBus::Dispatcher::create();
		DBus::Connection::pointer conn = dispatcher->create_connection(DBus::BUS_SESSION);
		int ret = conn->request_name("Firefinch.Network.Service.Mobile", DBUS_NAME_FLAG_REPLACE_EXISTING);
		if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret)
			return 1;
		mobileManager::pointer serverPtr = mobileManager::create();
		conn->register_object(serverPtr);

		serverPtr->initDevice();

		if (!_showHelp)
		{
			waitForTerminationRequest();
		}

		return Application::EXIT_OK;
	}

private:
	ErrorHandler _errorHandler;
	bool _showHelp;
};


POCO_SERVER_MAIN(mobileService)
