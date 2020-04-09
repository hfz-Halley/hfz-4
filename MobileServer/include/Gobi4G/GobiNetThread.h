#ifndef _GobiNetThread_H
#define _GobiNetThread_H

#include <Poco/Mutex.h>
#include <Poco/Thread.h>
#include "Poco/Event.h"
#include <Poco/BasicEvent.h>
#include "Poco/Logger.h"

using Poco::BasicEvent;
using Poco::Thread;

namespace Firefinch
{

namespace Net
{

class GobiNethread : public Poco::RefCountedObject, public Poco::Runnable
{

public:
	Poco::BasicEvent<int> theEvent;

	GobiNethread();
	~GobiNethread();
	void run();
	void start();
	void stop();
	void sendNotify(int triger_event)
	{
		theEvent.notifyAsync(this, triger_event);
	}

	int GobiNetGetClientID(const char *qcqmi, unsigned char QMIType);
	int GobiNetDeInit(void);

private:
	Thread _GobiNethread;
	bool running;
	Poco::Logger &_logger;
};

} // namespace Net

} // namespace Firefinch

#endif
