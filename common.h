#pragma once
#include "logger.hpp"

#include <stdio.h>

#ifdef _DEBUG
// #  define DEBUG_PRINT(...) printf(__VA_ARGS__)
#  define DEBUG_PRINT(...) LOGF(DEBUG, __VA_ARGS__)
#else
#  define DEBUG_PRINT(...)
// #  define DEBUG_PRINT(...) LOGF(DEBUG, __VA_ARGS__)
#endif
