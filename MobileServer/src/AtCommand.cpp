
#include "stdio.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "Poco/TextIterator.h"
#include "Poco/UTF8Encoding.h"
#include "Poco/TextConverter.h"
#include "AtCommand.h"

#define READ_SIZE 64
#define USB_DEVICE "/dev/ttyUSB2"
using Poco::TextIterator;
using Poco::UTF8Encoding;

namespace Poco
{
namespace Serial
{

ATcommand::ATcommand()
{
	com = new SerialPort(USB_DEVICE, 115200, "8N1"); //The port default open
	recvTimeout = (3 * Timespan::SECONDS);

}
ATcommand::~ATcommand()
{
	printf("~ATcommand com close\n");
	AT_ComClose();
}
bool ATcommand::AT_SendCmd(char *cmd)
{
	char SendData[32];
	sprintf(SendData, "%s\r\n", cmd);
	if (com->write(SendData, 32))
	{
		return true;
	}
	return false;
}




bool ATcommand::AT_Send(char *cmd, char *buffer, int size)
{
	char SendData[32];
	sprintf(SendData, "%s\r\n", cmd);
	if (com->write(SendData, 32))
	{
		sleep(1);
		if (com->poll(recvTimeout))
		{
			if (com->read(buffer, size) > 0)
			{
				printf("AT_Send:%s \n", buffer);
				return true;
			}
		}
	}
	return false;
}


bool ATcommand::simDetectd()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+QSIMDET=1,1", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
			return 2;
	}
	return 1;

}


bool ATcommand::getcpsi()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CPSI?", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
			return 2;
	}
	return 1;

}


bool ATcommand::AT_ComClose()
{
	com->close();
	if (com)
		delete com;

	return true;
}
bool ATcommand::AT_RecvData(char *rdata, int rsize)
{
	if (com->poll(recvTimeout))
	{
		if (com->read(rdata, rsize) > 0)
			return true;
	}
	return false;
}

bool ATcommand::GetRssidata()
{
	memset(RssiBuffer, 0, rssisize);
	if (AT_Send((char *)"AT+CSQ", RssiBuffer, rssisize))
	{
		if (strstr(RssiBuffer, "OK"))
		{
			return true;
		}
	}
	return false;
}

bool ATcommand::Setecho(bool enable)
{
	char echoBuffer[READ_SIZE];
	char echo[10];
	if (enable)
		sprintf(echo, "%s", "ATE1");
	else
		sprintf(echo, "%s", "ATE0");
	for (int i = 0; i < 3; i++)
	{
		if (!AT_Send(echo, echoBuffer, READ_SIZE))
		{
			return false;
		}
		if (strstr(echoBuffer, "OK") || strstr(echoBuffer, "ATE0"))
		{
			return true;
		}
	}
	return false;
}

int ATcommand::getSIMStatus()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CPIN?", buf, READ_SIZE))
	{
		if (strstr(buf, "READY"))
			return 2;
	}
	return 1;
}

/****************************
8:LTE mode  其他：
公网下 3GPP 模式（GSM/TDS/WCDMA/LTE）
AT$QCRMCALL=1,1 //拨号
AT$QCRMCALL=0,1 //挂断
************************************/

int ATcommand::getNetworkMode()
{
	char buf[READ_SIZE] = {0};
	int mode1, mode2;
	char str[10];
	if (!AT_Send((char *)"AT+CNSMOD?", buf, READ_SIZE))
	{
		return -1;
	}
	char *dest = strstr(buf, "+CNSMOD:");
	if (!dest)
		return -1;
	sscanf(dest, "%s %d,%d", str, &mode1, &mode2);
	if (mode2 == 8)
		return 1;
	return 0;
}

/************************************************
“1（或5）”表示数据业务
已经附着， 可以使用数据业务相关的功能了， 只有1和5是正确的；
“0/2/3/4” 表示数据业务未附着，模块还在和网络协商
附着状态中， 需要继续查询等待。
*************************************************/

int ATcommand::getNetAttachStatus()
{
	char buf[READ_SIZE] = {0};
	int arg1, arg2;
	char str[10];

	if (!AT_Send((char *)"AT+CGREG?", buf, READ_SIZE))
	{
		return -1;
	}
	char *dest = strstr(buf, "+CGREG:");
	if (!dest)
		return -1;
	sscanf(dest, "%s %d,%d", str, &arg1, &arg2);
	if (arg2 == 1 || arg2 == 5)
		return 1;
	return 0;
}

int ATcommand::GetCurSignalStrenght()
{
	char str1[10];
	int icsq1, icsq2;
	int signalTrans = 0;
	char *dest = strstr(RssiBuffer, "+CSQ: ");
	if (!dest)
		return -1;

	sscanf(dest, "%s %d,%d", str1, &icsq1, &icsq2);

	signalTrans = -113 + (2 * icsq1);
	printf("signalTrans=%d \n",signalTrans);
	if (signalTrans >= -51)
		return GREAT;
	else if (signalTrans < -51 && signalTrans > -70)
	{
		return GOOD;
	}
	else if (signalTrans < -70 &&  signalTrans > -80)
	{
		return MODERATE;
	}
	else if (signalTrans < -80 && signalTrans > -100)
	{
		return POOR;
	}
	else
		return UNKNOWN;
}

bool ATcommand::SetMsmformat()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CMGF=1", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}
	return false;
}


bool ATcommand::set_cnsmod()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CNSMOD=1", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}
	return false;

}


bool ATcommand::setKeepalive()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CTCPKA=1,2,10", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}
	return false;

}


bool ATcommand::SetCharacter()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CSCS=\"GSM\"", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}
	return false;
}

bool ATcommand::setphonenum(char *phonenum)
{
	char send_number[16] = {0};
	char RecvData[READ_SIZE] = {0};
	sprintf(send_number, "AT+CMGS=\"%s\"", phonenum);

	if (AT_Send(send_number, RecvData, READ_SIZE))
	{
		if (strstr(RecvData, ">"))
		{
			return true;
		}
	}
	return false;
}

bool ATcommand::sendTextmessage(char *msg)
{
	char Msg[512] = {0};
	char RecvData[128] = {0};
	sprintf(Msg, "%s", msg);
	strcat(Msg, "\x1a");

	if (com->write(Msg, strlen(Msg)))
	{
		printf("write %s\n", Msg);
		if (AT_RecvData(RecvData, 128))
		{
			//printf("read %s\n", RecvData);
			if (strstr(RecvData, "OK"))
			{
				return true;
			}
		}
	}
	return false;
}

bool ATcommand::SetPDUmode()
{
	char buf[READ_SIZE] = {0};
	if (AT_Send((char *)"AT+CMGF=0", buf, READ_SIZE))
	{
		if (strstr(buf, "OK"))
		{
			return true;
		}
	}

	return false;
}

bool ATcommand::phonenum_parity_exchange(char *indata, char *outdata)
{

	int i = 0;
	int len = strlen(indata);
	char tmp[16] = {0};
	int ret = len % 2;
	if (!indata)
		return false;
	memcpy(tmp, indata, 16);
	if (ret != 0)
	{
		strcat(tmp, "f");
		len = len + 1;
	}
	for (i = 0; i < len; i += 2)
	{
		outdata[i] = tmp[i + 1];
		outdata[i + 1] = tmp[i];
	}
	return true;
}

bool ATcommand::get_sms_center(char *sms_center)
{
	char RecvData[READ_SIZE * 2] = {0};
	char tmp[16] = {0};
	char *num_start = NULL, *num_end = NULL;
	int num_len;

	if (AT_Send((char *)"AT+CSCA?", RecvData, READ_SIZE * 2))
	{
		//printf("read %s\n",RecvData);
		num_start = strchr(RecvData, '"');
		if (num_start == NULL)
			return false;
		else
			*num_start = '\0';

		//printf("rdata num:%s\n", num_start+1);
		num_end = strchr(num_start + 1, '"');
		if (num_end == NULL)
			return false;
		else
			*num_end = '\0';
		//printf("rdata start:%s\n", num_start+2);
		phonenum_parity_exchange(num_start + 2, tmp);

		num_len = (strlen(tmp) + 2) / 2;
		sprintf(sms_center, "%02x%s%s", num_len, "91", tmp);
		return true;
	}
	return false;
}

bool ATcommand::get_sms_dest(char *phonenum, char *dest)
{
	char tmp[16] = {0};
	int num_len = strlen(phonenum);
	if (strchr(phonenum, '+'))
	{
		num_len = num_len - 1;
		phonenum_parity_exchange(phonenum + 1, tmp);
		sprintf(dest, "%s%02x%s", "1100", num_len, "91");
	}
	else
	{
		phonenum_parity_exchange(phonenum, tmp);
		sprintf(dest, "%s%02x%s", "1100", num_len, "81");
	}
	strcat(dest, tmp);
	strcat(dest, "000800");
	return true;
}

bool ATcommand::ConverToUnicode(std::string utf8String, char *outdata)
{
	UTF8Encoding utf8;
	TextIterator it(utf8String, utf8);
	TextIterator end(utf8String);
	char buf[1024] = {0};
	for (; it != end; ++it)
	{
		sprintf(buf + strlen(buf), "%04x", *it);
	}
	sprintf(outdata, "%02x", strlen(buf) / 2);
	strcat(outdata, buf);
	//printf("buf==%s\n",outdata);
	return true;
}

bool ATcommand::sendPduMessage(std::string phonenum, std::string utf8Msg)

{
	char sms_center[32] = {0};
	char phonenum_dest[32] = {0};
	char UnicodeMsg[256] = {0};
	char Senddata[256] = {0};
	char CMGS[16] = {0};
	char recvdata[64] = {0};
	int Msglen = 0;

	if (!SetPDUmode())
	{
		return false;
	}
	get_sms_center(sms_center);
	get_sms_dest((char *)phonenum.data(), phonenum_dest);
	ConverToUnicode(utf8Msg, UnicodeMsg);
	//set sms number
	Msglen = strlen(phonenum_dest) + strlen(UnicodeMsg);
	sprintf(CMGS, "AT+CMGS=%d", Msglen / 2);
	printf("CMGS = %s\n", CMGS);
	if (!AT_SendCmd(CMGS))
	{
		return false;
	}
	sleep(2);

	//send sms
	sprintf(Senddata, "%s%s%s", sms_center, phonenum_dest, UnicodeMsg);
	strcat(Senddata, "\x1a");
	if (com->write(Senddata, strlen(Senddata)))
	{
		sleep(2);
		printf("write %s\n",Senddata);
		if (AT_RecvData(recvdata, 64))
		{
			printf("read %s\n", recvdata);
			if (strstr(recvdata, "OK"))
			{
				return true;
			}
		}
	}
	return false;
}

} // namespace Serial

} // namespace Poco
