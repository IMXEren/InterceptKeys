#include "logger.hpp"

#ifdef _DEBUG

// #include "quill/Backend.h"
// #include "quill/Frontend.h"
// #include "quill/Logger.h"
// #include "quill/sinks/RotatingFileSink.h"

// quill::Logger* gLogger = nullptr;

void initLogging() {
	// quill::Backend::start();
	// gLogger = quill::Frontend::create_or_get_logger(
	//     "root", quill::Frontend::create_or_get_sink<quill::RotatingFileSink>(
	//                 LOGFILE_PATH, []() {
	//                   quill::RotatingFileSinkConfig cfg;
	//                   cfg.set_open_mode('w');
	//                   cfg.set_filename_append_option(
	//                       quill::FilenameAppendOption::StartDateTime);
	//                   cfg.set_rotation_max_file_size(10 * 1024 * 1024);  // 10MB
	//                   return cfg;
	//                 }()));

	std::string rotateFileLog = "intercept_keys";
	std::string directory = LOGFILE_DIR;

	auto logworker = g3::LogWorker::createLogWorker();
	auto sinkHandle = logworker->addSink(
		std::make_unique<LogRotate>(rotateFileLog, directory), &LogRotate::save);
	g3::initializeLogging(logworker.get());

	size_t maxBytesBeforeRotatingFile = 1000000;  // 10 MB
	sinkHandle->call(&LogRotate::setMaxLogSize, maxBytesBeforeRotatingFile)
		.wait();
}

#endif // _DEBUG