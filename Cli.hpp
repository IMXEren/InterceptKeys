
#pragma once
#include "config.h"
#include "CLI11.hpp"
#include "Service.hpp"
#include "MapConfig.hpp"
#include "common.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>



class Cli {
	static bool filter_existing_files(const std::string& config_path) {
		std::error_code ec;
		auto path = std::filesystem::canonical(config_path, ec);
		if (ec) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Invalid path: '{}'. {}", config_path, ec.message());
			return false;
		}
		return true;
	};

public:
	enum { RUN_AS_SERVICE, RUN_AS_CONSOLE };
	bool status_flag = false;
	bool detect_keys_only_flag = false;
	int parse_and_run(int argc, char* argv[]) {
		CLI::App gApp{ "Intercept key strokes to send modified key strokes using interception kernel-mode driver.", "InterceptKeys" };
		gApp.set_version_flag("-v, --version", INTERCEPT_KEYS_VERSION);

		auto install_command = gApp.add_subcommand("install", "Install the InterceptKeysService.");
		auto start_command = gApp.add_subcommand("start", "Start the InterceptKeysService.");
		auto stop_command = gApp.add_subcommand("stop", "Stop the InterceptKeysService.");
		auto restart_command = gApp.add_subcommand("restart", "Restart the InterceptKeysService.");
		auto uninstall_command = gApp.add_subcommand("uninstall", "Uninstall the InterceptKeysService.");

		gApp.add_flag("--status", status_flag, "Fetch the running status of InterceptKeysService.");
		gApp.add_flag("--detect-keys-only", detect_keys_only_flag, "Helper to detect key clicks to find the necessary scancode.");
		gApp.add_flag("--service", SERVICE_FLAG, "Run the interception as a service. Recommended after you've setup the map config.");

		std::filesystem::path log_directory = std::filesystem::current_path() / "logs";
		auto logging_option = gApp.add_option("--logging", log_directory, "Enable logging")
			->option_text("[DIR] [default: logs]")
			->default_str(log_directory.string())
			->expected(0, 1);

		std::vector<std::string> map_config = { "mapping.toml" };
		auto map_config_option = gApp.add_option_function<std::vector<std::string>>("--map-config",
			[&](const std::vector<std::string>& config_path_list) {
				map_config.clear();
				std::copy_if(config_path_list.begin(), config_path_list.end(), std::back_inserter(map_config), Cli::filter_existing_files);
			},
			"List of mapping config file to use while Interception.")
			->expected(-1)
			->option_text("[TOML]... [default: mapping.toml]");

		install_command->add_option(map_config_option->get_name(), map_config, map_config_option->get_description())
			->expected(map_config_option->get_expected())
			->check(CLI::ExistingFile) // For install, check if existing else raise error
			->option_text(map_config_option->get_option_text());

		auto ilogging_option = install_command->add_option(logging_option->get_name(), log_directory, logging_option->get_description())
			->option_text(logging_option->get_option_text())
			->default_str(logging_option->get_default_str())
			->expected(0, 1);

		try {
			gApp.parse(argc, argv);
			INTERCEPT_LOGD_N_OUT(gLogger, "Parsed args!");
		}
		catch (const CLI::ParseError& e) {
			int code = gApp.exit(e);
			exit(code);
		}

		if (*ilogging_option || *logging_option) {
			std::error_code ec;
			std::filesystem::create_directories(log_directory, ec);
			if (ec) {
				INTERCEPT_EPRINTLN("Failed to create log directory: {}. {}", log_directory.string(), ec.message());
				exit(ec.value());
			}
			LOGGING_FLAG = true;
			initLogging(log_directory);
			INTERCEPT_LOGD_N_OUT(gLogger, "Using log directory: {}", log_directory.string());
		}

		for (auto& config_path : map_config) {
			auto path = std::filesystem::canonical(config_path);
			INTERCEPT_LOGD_N_OUT(gLogger, "Using map config: {}", path.string());
			config_path = path.string();
		}

		if (gApp.got_subcommand(install_command)) {
			INTERCEPT_LOGD_N_OUTW(gLogger, "Installing InterceptKeysService...");
			std::vector<std::string> args = {};
			args.push_back("--service");
			if (LOGGING_FLAG) {
				args.push_back(logging_option->get_name());
				args.push_back(log_directory.string());
			}
			args.push_back(map_config_option->get_name());
			args.reserve(args.size() + distance(map_config.begin(), map_config.end()));
			args.insert(args.end(), map_config.begin(), map_config.end());
			install(args);
		}
		else if (gApp.got_subcommand(start_command)) {
			INTERCEPT_LOGD_N_OUT(gLogger, "Starting InterceptKeysService...");
			start();
		}
		else if (gApp.got_subcommand(stop_command)) {
			INTERCEPT_LOGD_N_OUT(gLogger, "Stopping InterceptKeysService...");
			stop();
		}
		else if (gApp.got_subcommand(restart_command)) {
			INTERCEPT_LOGD_N_OUT(gLogger, "Restarting InterceptKeysService...");
			restart();
		}
		else if (gApp.got_subcommand(uninstall_command)) {
			INTERCEPT_LOGD_N_OUT(gLogger, "Uninstalling InterceptKeysService...");
			uninstall();
		}
		else if (status_flag == true) {
			Service intercept_service;
			DWORD code = intercept_service.get_service_status();
			exit(code);
		}

		int run = RUN_AS_CONSOLE;
		if (SERVICE_FLAG)
			run = RUN_AS_SERVICE;

		MapConfig mc(std::move(map_config));
		mc.load_config();
		return run;
	}

	[[noreturn]]
	static void install(const std::vector<std::string>& exe_args) {
		Service intercept_service;
		BOOL ok = intercept_service.create_or_update_service(exe_args);
		if (!ok) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to install the service.");
			exit(1);
		}
		ok = intercept_service.start_service();
		exit(~ok);
	}

	[[noreturn]]
	static void start() {
		Service intercept_service;
		BOOL ok = intercept_service.start_service();
		exit(~ok);
	}

	[[noreturn]]
	static void stop() {
		Service intercept_service;
		BOOL ok = intercept_service.stop_service();
		exit(~ok);
	}

	[[noreturn]]
	static void restart() {
		Service intercept_service;
		BOOL ok = intercept_service.stop_service();
		if (ok) {
			ok = intercept_service.start_service();
			exit(~ok);
		}
		exit(~ok);
	}

	[[noreturn]]
	static void uninstall() {
		Service intercept_service;
		BOOL ok = intercept_service.delete_service();
		exit(~ok);
	}
};

inline Cli gCli;