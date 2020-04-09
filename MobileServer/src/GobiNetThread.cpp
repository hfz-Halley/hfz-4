#include "GobiNetThread.h"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <ctype.h>
#include "QMIThread.h"

// IOCTL to generate a client ID for this service type
#define IOCTL_QMI_GET_SERVICE_FILE 0x8BE0 + 1
//IOCTL to get the VIDPID of the device
#define IOCTL_QMI_GET_DEVICE_VIDPID 0x8BE0 + 2
// IOCTL to get the MEID of the device
#define IOCTL_QMI_GET_DEVICE_MEID 0x8BE0 + 3

namespace Firefinch
{

namespace Net
{

#define TAG "GobiNethread"
GobiNethread::GobiNethread() : _logger(Poco::Logger::get(TAG))
{
	running = false;
}

GobiNethread::~GobiNethread()
{
	GobiNetDeInit();
	QmiThreadRecvQMI(NULL, NULL);
	// sendNotify(-1);
}

void GobiNethread::start()
{
	_GobiNethread.start(*this);
}
void GobiNethread::stop()
{
	running = true;
	if (_GobiNethread.isRunning())
	{
		_GobiNethread.join();
	}
}

void GobiNethread::run()
{
	const char *qcqmi = "/dev/qcqmi2";
	qmiclientId[QMUX_TYPE_WDS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_WDS); //申请客户端iD
	qmiclientId[QMUX_TYPE_DMS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_DMS);
	qmiclientId[QMUX_TYPE_NAS] = GobiNetGetClientID(qcqmi, QMUX_TYPE_NAS);
	qmiclientId[QMUX_TYPE_UIM] = GobiNetGetClientID(qcqmi, QMUX_TYPE_UIM);
	qmiclientId[QMUX_TYPE_WDS_ADMIN] = GobiNetGetClientID(qcqmi, QMUX_TYPE_WDS_ADMIN);
	//donot check clientWDA, there is only one client for WDA, if quectel-CM is killed by SIGKILL, i cannot get client ID for WDA again!
	if (qmiclientId[QMUX_TYPE_WDS] == 0)
	{
		GobiNetDeInit();
		dbg_time("%s Failed to open %s, errno: %d (%s)", __func__, qcqmi, errno, strerror(errno));
		sendNotify(RIL_INDICATE_DEVICE_DISCONNECTED);
	}
	sendNotify(RIL_INDICATE_DEVICE_CONNECTED);
	while (!running)
	{
		struct pollfd pollfds[16] = {};
		int ne, ret, nevents = 1;
		unsigned int i;

		for (i = 0; i < sizeof(qmiclientId) / sizeof(qmiclientId[0]); i++)
		{
			if (qmiclientId[i] != 0)
			{
				pollfds[nevents].fd = qmiclientId[i];
				pollfds[nevents].events = POLLIN;
				pollfds[nevents].revents = 0;
				nevents++;
			}
		}

		do
		{
			ret = poll(pollfds, nevents, 1000);

		} while ((ret < 0) && (errno == EINTR));

		//ret = poll(pollfds, nevents, -1);
		if (ret > 0)
		{

			for (ne = 0; ne < nevents; ne++)
			{

				int fd = pollfds[ne].fd;
				short revents = pollfds[ne].revents;
				if (revents & (POLLERR | POLLHUP | POLLNVAL))
				{
					dbg_time("%s poll err/hup/inval", __func__);
					dbg_time("epoll fd = %d, events = 0x%04x", fd, revents);
					if (revents & (POLLERR | POLLHUP | POLLNVAL))
					{
						break;
					}
				}
				if ((revents & POLLIN) == 0)
				{
					continue;
				}
				ssize_t nreads;
				UCHAR QMIBuf[512];
				PQCQMIMSG pResponse = (PQCQMIMSG)QMIBuf;

				nreads = read(fd, &pResponse->MUXMsg, sizeof(QMIBuf) - sizeof(QCQMI_HDR));
				if (nreads <= 0)
				{
					dbg_time("%s read=%d errno: %d (%s)", __func__, (int)nreads, errno, strerror(errno));
					break;
				}
				for (i = 0; i < sizeof(qmiclientId) / sizeof(qmiclientId[0]); i++)
				{
					if (qmiclientId[i] == fd)
					{
						pResponse->QMIHdr.QMIType = i;
					}
				}
				pResponse->QMIHdr.IFType = USB_CTL_MSG_TYPE_QMI;
				pResponse->QMIHdr.Length = cpu_to_le16(nreads + sizeof(QCQMI_HDR) - 1);
				pResponse->QMIHdr.CtlFlags = 0x00;
				pResponse->QMIHdr.ClientId = fd & 0xFF;

				sendNotify(QmiThreadRecvQMI(NULL, pResponse));
			}
		}
	}
}

int GobiNethread::GobiNetGetClientID(const char *qcqmi, unsigned char QMIType)
{
	int ClientId;
	ClientId = open(qcqmi, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (ClientId == -1)
	{
		dbg_time("failed to open %s, errno: %d (%s)", qcqmi, errno, strerror(errno));
		return -1;
	}
	if (ioctl(ClientId, IOCTL_QMI_GET_SERVICE_FILE, QMIType) != 0)
	{
		dbg_time("failed to get ClientID for 0x%02x errno: %d (%s)", QMIType, errno, strerror(errno));
		close(ClientId);
		ClientId = 0;
	}
	switch (QMIType)
	{
	case QMUX_TYPE_WDS:
		dbg_time("Get clientWDS = %d", ClientId);
		break;
	case QMUX_TYPE_DMS:
		dbg_time("Get clientDMS = %d", ClientId);
		break;
	case QMUX_TYPE_NAS:
		dbg_time("Get clientNAS = %d", ClientId);
		break;
	case QMUX_TYPE_QOS:
		dbg_time("Get clientQOS = %d", ClientId);
		break;
	case QMUX_TYPE_WMS:
		dbg_time("Get clientWMS = %d", ClientId);
		break;
	case QMUX_TYPE_PDS:
		dbg_time("Get clientPDS = %d", ClientId);
		break;
	case QMUX_TYPE_UIM:
		dbg_time("Get clientUIM = %d", ClientId);
		break;
	case QMUX_TYPE_WDS_ADMIN:
		dbg_time("Get clientWDA = %d", ClientId);
		break;
	default:
		break;
	}

	return ClientId;
}
int GobiNethread::GobiNetDeInit(void)
{
	unsigned int i;
	for (i = 0; i < sizeof(qmiclientId) / sizeof(qmiclientId[0]); i++)
	{
		if (qmiclientId[i] != 0)
		{
			close(qmiclientId[i]);
			qmiclientId[i] = 0;
		}
	}

	return 0;
}

} // namespace Net

} // namespace Firefinch
