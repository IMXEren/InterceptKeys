#pragma once
#include "quill/Logger.h"
#include "quill/LogMacros.h"
#include "quill/std/WideString.h"

#include <filesystem>

extern quill::Logger* gLogger;
void initLogging(const std::filesystem::path& log_dir);
