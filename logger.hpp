#pragma once
#include "quill/Logger.h"
#include "quill/LogMacros.h"
#include "quill/std/WideString.h"

#include <filesystem>

extern quill::Logger* g_Logger;
void initLogging(const std::filesystem::path& log_dir);
