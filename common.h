#pragma once
#include "logger.hpp"
#include <iostream>

inline bool g_ServiceFlag = false;
inline bool g_LoggingFlag = false;

#define INTERCEPT_LOG_DEBUG(logger, fmt, ...) do {			\
	if (g_LoggingFlag) {										\
		LOG_DEBUG(g_Logger, fmt, __VA_ARGS__);				\
	}														\
} while (0)

#define INTERCEPT_LOG_ERROR(logger, fmt, ...) do {			\
	if (g_LoggingFlag) {										\
		LOG_ERROR(g_Logger, fmt, __VA_ARGS__);				\
	}														\
} while (0)

#define INTERCEPT_PRINTLN(_fmt, ...) do {					\
	if (!g_ServiceFlag) {									\
		fmt::println(_fmt, __VA_ARGS__);					\
	}														\
} while (0)

#define INTERCEPT_EPRINTLN(_fmt, ...) do {					\
	if (!g_ServiceFlag) {									\
		fmt::println(std::cerr, _fmt, __VA_ARGS__);			\
	}														\
} while (0)

#define INTERCEPT_EPRINTLNW(_fmt, ...) do {					\
	if (!g_ServiceFlag) {									\
		fmt::println(std::wcerr, _fmt, __VA_ARGS__);		\
	}														\
} while (0)

#define INTERCEPT_LOGD_N_OUT(logger, fmt, ...) do {			\
	INTERCEPT_LOG_DEBUG(logger, fmt, __VA_ARGS__);			\
	INTERCEPT_PRINTLN(fmt, __VA_ARGS__);					\
} while (0)

#define INTERCEPT_LOGD_N_OUTW(logger, fmt, ...) do {		\
	INTERCEPT_LOG_DEBUG(logger, fmt, __VA_ARGS__);			\
	INTERCEPT_PRINTLN(L##fmt, __VA_ARGS__);					\
} while (0)

#define INTERCEPT_LOGE_N_ERR(logger, fmt, ...) do {			\
	INTERCEPT_LOG_ERROR(logger, fmt, __VA_ARGS__);			\
	INTERCEPT_EPRINTLN(fmt, __VA_ARGS__);					\
} while (0)

#define INTERCEPT_LOGE_N_ERRW(logger, fmt, ...) do {		\
	INTERCEPT_LOG_ERROR(logger, fmt, __VA_ARGS__);			\
	INTERCEPT_EPRINTLNW(L##fmt, __VA_ARGS__);				\
} while (0)
