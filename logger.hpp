#pragma once

#ifdef _DEBUG

// #include "quill/LogMacros.h"
// #include "quill/Logger.h"

#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"
#include "g3sinks/LogRotate.h"

#define LOGFILE_DIR  "D:\\CodeDev\\VisualStudio\\InterceptKeys\\logs"
#define LOGFILE_PATH LOGFILE_DIR "\\intercept_keys.log"

// extern quill::Logger* gLogger;
void initLogging();

#else
#define initLogging(...)
#endif // _DEBUG
