#include "logger.hpp"

#ifdef ENABLE_LOGGING

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/Logger.h"
#include "quill/sinks/RotatingFileSink.h"

#define LOGFILE_DIR  __FILE__ "\\..\\logs"
#define LOGFILE_PATH LOGFILE_DIR "\\intercept_keys.log"

quill::Logger* gLogger = nullptr;

void initLogging() {
	if (gLogger != nullptr) return;
	quill::Backend::start();
	gLogger = quill::Frontend::create_or_get_logger(
		"root", quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
			LOGFILE_PATH, []() {
				quill::RotatingFileSinkConfig cfg;
				cfg.set_open_mode('w');
				cfg.set_filename_append_option(
					quill::FilenameAppendOption::StartDateTime);
				cfg.set_rotation_max_file_size(10 * 1024 * 1024);  // 10MB
				return cfg;
			}()));
	gLogger->set_log_level(quill::LogLevel::Debug);
}

#endif // ENABLE_LOGGING