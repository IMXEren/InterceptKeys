#pragma once
#include "logger.hpp"

#ifndef ENABLE_LOGGING
#  define LOG_DEBUG(...)
#endif

#ifdef BUILD_AS_SERVICE
#  define DEBUG_PRINT(...)
#else
#  define DEBUG_PRINT(...) printf(__VA_ARGS__)
#endif
