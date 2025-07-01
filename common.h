#pragma once
#include "logger.hpp"

inline bool SERVICE_FLAG = false;
inline bool LOGGING_FLAG = false;

#define INTERCEPT_LOG_DEBUG(logger, fmt, ...) do {			\
	if (LOGGING_FLAG) {										\
		LOG_DEBUG(gLogger, fmt, __VA_ARGS__);				\
	}														\
} while (0)

#define INTERCEPT_LOG_ERROR(logger, fmt, ...) do {			\
	if (LOGGING_FLAG) {										\
		LOG_ERROR(gLogger, fmt, __VA_ARGS__);				\
	}														\
} while (0)

#define INTERCEPT_PRINTLN(_fmt, ...) do {					\
	if (!SERVICE_FLAG) {									\
		fmt::println(_fmt, __VA_ARGS__);					\
	}														\
} while (0)

#define INTERCEPT_EPRINTLN(_fmt, ...) do {					\
	if (!SERVICE_FLAG) {									\
		fmt::println(std::cerr, _fmt, __VA_ARGS__);			\
	}														\
} while (0)

#define INTERCEPT_EPRINTLNW(_fmt, ...) do {					\
	if (!SERVICE_FLAG) {									\
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
