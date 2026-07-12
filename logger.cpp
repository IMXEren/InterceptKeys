#include "logger.hpp"
#include <filesystem>

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/Logger.h"
#include "quill/sinks/RotatingFileSink.h"

quill::Logger* g_Logger = nullptr;

void initLogging(const std::filesystem::path& log_dir) {
	if (g_Logger != nullptr) return;
	auto log_file = log_dir / "intercept_keys.log";
	quill::Backend::start();
	g_Logger = quill::Frontend::create_or_get_logger(
		"root", quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
			log_file.string(), []() {
				quill::RotatingFileSinkConfig cfg;
				cfg.set_open_mode('w');
				cfg.set_filename_append_option(
					quill::FilenameAppendOption::StartDateTime);
				cfg.set_rotation_max_file_size(10 * 1024 * 1024);  // 10MB
				cfg.set_max_backup_files(15);                     // keep 15 rotated logs
				return cfg;
			}()));
	g_Logger->set_log_level(quill::LogLevel::Debug);
}
