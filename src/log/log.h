#pragma once

#include "../engine_api/RtcLog.h"

namespace live_stream_sdk {
    
const char* GetLiveSDKDeviceID();
void SetLiveSDKDeviceID(const char* deviceid);

void LogWebrtcPrintf(int level, const char *file, int line_num, const char *format, va_list args);
void LogNetworkPrintf(int level, const char *file, int line_num, const char *format, ...);

}

