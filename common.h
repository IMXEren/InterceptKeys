#pragma once
#include "logger.hpp"

#ifndef ENABLE_LOGGING
#  define LOG_DEBUG(...)
#endif

#ifdef BUILD_AS_SERVICE
#  define PRINT_IF_NOT_SERVICE(...)
#else
#  define PRINT_IF_NOT_SERVICE(...) printf(__VA_ARGS__)
#endif
