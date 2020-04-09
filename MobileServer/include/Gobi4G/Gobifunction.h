#ifndef __Gobifunction__H
#define __Gobifunction__H

#include "QMIThread.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern int qmidevice_detect(char **pp_qmichannel, char **pp_usbnet_adapter);
    extern int kill_brothers(PROFILE_T *profile);

#ifdef __cplusplus
}
#endif

#endif
