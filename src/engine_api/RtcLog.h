#pragma once

#include <stdarg.h>
#include <stdio.h>

#ifndef DLLEXPORT
#ifdef WIN32
#ifdef ENGINE_SDK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif
#endif

namespace live_stream_sdk {
DLLEXPORT void LogSetDir(const char *dir, void(*callback)(const char *msg) = NULL);

// 下面的都只是作为调试用途
enum LogLevel {
  LOG_LEVEL_TRC = 1,
  LOG_LEVEL_DBG = 5,
  LOG_LEVEL_INF = 6,
  LOG_LEVEL_WRN = 7,
  LOG_LEVEL_ERR = 8,
  LOG_LEVEL_NON = 9,
};
DLLEXPORT void LogPrintf(int level, const char *file, int line_num, const char *format, ...);
DLLEXPORT LogLevel LogGetLevel();
DLLEXPORT void LogSetLevel(LogLevel level);
}

#define TRC(...) \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_TRC) { \
  live_stream_sdk::LogPrintf(live_stream_sdk::LOG_LEVEL_TRC, __FILE__, __LINE__, __VA_ARGS__); \
}

#define DBG(...)  \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_DBG) { \
  live_stream_sdk::LogPrintf(live_stream_sdk::LOG_LEVEL_DBG, __FILE__, __LINE__, __VA_ARGS__); \
}

#define INF(...) \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_INF) { \
  live_stream_sdk::LogPrintf(live_stream_sdk::LOG_LEVEL_INF, __FILE__, __LINE__, __VA_ARGS__); \
}

#define WRN(...) \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_WRN) { \
  live_stream_sdk::LogPrintf(live_stream_sdk::LOG_LEVEL_WRN, __FILE__, __LINE__, __VA_ARGS__); \
}

#define ERR(...) \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_ERR) { \
  live_stream_sdk::LogPrintf(live_stream_sdk::LOG_LEVEL_ERR, __FILE__, __LINE__, __VA_ARGS__); \
}

#define TRC_NET(...) \
if (live_stream_sdk::LogGetLevel() <= live_stream_sdk::LOG_LEVEL_TRC) { \
  live_stream_sdk::LogNetworkPrintf(live_stream_sdk::LOG_LEVEL_TRC, __FILE__, __LINE__, __VA_ARGS__);	\
}
