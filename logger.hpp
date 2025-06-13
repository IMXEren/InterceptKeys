#pragma once

#ifdef ENABLE_LOGGING

#include "quill/LogMacros.h"
#include "quill/Logger.h"

extern quill::Logger* gLogger;
void initLogging();


#else

#define initLogging(...)

#endif // _DEBUG
