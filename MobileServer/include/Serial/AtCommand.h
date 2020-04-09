#ifndef _ATcommand_H
#define _ATcommand_H
#include "SerialPort.h"
#include <iconv.h>

using Poco::Timespan;
#define rssisize 128

namespace Poco
{
namespace Serial
{

class ATcommand
{

public:
	enum signaLevel
	{
		UNKNOWN = 0, //无信号
		POOR,
		MODERATE,
		GOOD,
		GREAT //满格
	};

	ATcommand();
	~ATcommand();

	bool AT_SendCmd(char *cmd);
	bool AT_Send(char *cmd, char *buffer, int size);
	bool AT_ComClose();
	bool AT_RecvData(char *rdata, int rsize);
	bool Setecho(bool enable);

	//RSSI
	bool GetRssidata();
	int GetCurSignalStrenght();

	bool getcpsi();
	bool set_cnsmod();
	
	bool setKeepalive();
	bool simDetectd();

	//check Network registration status
	int getSIMStatus();
	int getNetworkMode();
	int getNetAttachStatus();

	//text meassage
	bool SetMsmformat();
	bool SetCharacter();
	bool setphonenum(char *phonenum);
	bool sendTextmessage(char *msg);

	//PDU meassage
	bool SetPDUmode();
	bool phonenum_parity_exchange(char *indata, char *outdata);
	bool get_sms_center(char *sms_center);
	bool get_sms_dest(char *phonenum, char *dest);
	bool ConverToUnicode(std::string utf8String, char *outdata);
	bool sendPduMessage(std::string phonenum, std::string utf8Msg);

private:
	SerialPort *com;
	Timespan recvTimeout;
	char RssiBuffer[rssisize];
};

} // namespace Serial

} // namespace Poco
#endif
