#ifndef _LF_API_DE2F735D_1073_4D4A_A088_50913234587C_
#define _LF_API_DE2F735D_1073_4D4A_A088_50913234587C_

#include "avengine_types.h"
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus

#ifdef WIN32
#ifdef ENGINE_SDK_EXPORTS
#define LF_API __declspec(dllexport)
#else
#define LF_API __declspec(dllimport)
#endif
#else
#define LF_API
#endif

LF_API lfrtcGlobalConfig* getAvengineParamsSDK();

#if defined(__ANDROID__)
LF_API int rtcAndroidInit(void* jvm, void* context);
LF_API int rtcAndroidUninit();
#endif

// just for test
LF_API void create_stream_sync(char *streamid, const char *url);
LF_API void http_get_sync(char *content, const char *url);

#endif

LF_API int rtcInit();
LF_API void rtcUninit();

#ifdef __cplusplus
}
#endif
#endif//_LF_API_DE2F735D_1073_4D4A_A088_50913234587C_
