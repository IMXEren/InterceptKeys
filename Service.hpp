#pragma once
#define NOMINMAX
#include <windows.h>

#include "common.h"
#include "utils.hpp"
#include "fmt/xchar.h"

#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#define INTERCEPT_GLOBAL_MUTEX L"Global\\InterceptKeysMutex"
extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_StatusHandle;
extern HANDLE g_ServiceStopEvent;

void WINAPI ServiceCtrlHandler(DWORD CtrlCode);
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
int InterceptKeys();

typedef struct {
	int argc;
	char** argv;
} ProgramArgs;

DWORD getGlobalMutex(HANDLE* hMutex);
void ConvertCommandLineArgs(DWORD dwArgc, LPTSTR* lpszArgv, int* argcOut, char*** argvOut);

class Service {
	SC_HANDLE _scm = nullptr;
	SC_HANDLE _service = nullptr;
public:
	static constexpr LPCWSTR SERVICE_NAME = L"InterceptKeysService";
	static constexpr LPCWSTR DISPLAY_NAME = L"InterceptKeys";
	static constexpr LPCWSTR DESCRIPTION =
		L"Intercept key strokes to send modified key strokes using interception kernel-mode driver. (https://github.com/IMXEren/InterceptKeys)";
	std::wstring BIN_PATH;

	Service() {
		load_exe_path();
	}

	~Service() {
		if (_service)
			CloseServiceHandle(_service);
		if (_scm)
			CloseServiceHandle(_scm);
	}

	int run_command(const std::wstring& command) {
		FILE* pipe = _wpopen((command + L" 2>&1").c_str(), L"r");
		if (!pipe) return -1;

		wchar_t buffer[512];
		while (fgetws(buffer, sizeof(buffer) / sizeof(wchar_t), pipe)) {
			std::wcout << buffer;
		}
		return _pclose(pipe);
	}

	bool ask_user_to_update() {
		std::wcout << L"\nDo you want to update the service? [y/n] ";
		wchar_t response;
		std::wcin >> response;
		return response == L'y' || response == L'Y';
	}

	void load_exe_path() {
		wchar_t path_buf[MAX_PATH] = { 0 };
		DWORD len = GetModuleFileNameW(nullptr, path_buf, MAX_PATH);
		DWORD err = GetLastError();
		if (len == 0 || err) {
			INTERCEPT_LOGE_N_ERRW(gLogger, "Failed to get executable path (len: {}): {}\nError: {}", len, std::wstring(path_buf), err);
			INTERCEPT_LOGE_N_ERR(gLogger, "Exiting...");
			exit(err);
		}
		BIN_PATH = path_buf;
	}

	SC_HANDLE get_service_manager() {
		if (_scm == nullptr) {
			_scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
			if (!_scm) {
				DWORD err = GetLastError();
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to open Service Control Manager. Error: {}", err);
				if (err == ERROR_ACCESS_DENIED) {
					INTERCEPT_LOGE_N_ERR(gLogger, "Please run this program inside a console executed as administrator.");
				}
			}
		}
		return _scm;
	}

	SC_HANDLE get_service() {
		if (_service == nullptr) {
			_scm = get_service_manager();
			assert(_scm && "Service Manager handle cannot be null");
			_service = OpenService(_scm, SERVICE_NAME, SERVICE_ALL_ACCESS);
			if (!_service) {
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to open existing service. Error: {}", GetLastError());
			}
		}
		return _service;
	}

	BOOL start_service() {
		SC_HANDLE service = get_service();
		assert(service != nullptr && "Service handle cannot be null");
		BOOL status = StartServiceW(service, 0, nullptr);
		if (!status) {
			DWORD err = GetLastError();
			if (err == ERROR_SERVICE_ALREADY_RUNNING) {
				INTERCEPT_LOGE_N_ERR(gLogger, "Service already running.");
			}
			else {
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to start service. Error: {}", err);
			}
		}
		else {
			INTERCEPT_LOGD_N_OUT(gLogger, "Service started successfully.");
		}
		return status;
	}

	BOOL stop_service() {
		SC_HANDLE service = get_service();
		assert(service != nullptr && "Service handle cannot be null");
		BOOL status = ControlService(service, SERVICE_CONTROL_STOP, &g_ServiceStatus);
		if (!status) {
			DWORD err = GetLastError();
			if (err == ERROR_SERVICE_NOT_ACTIVE) {
				INTERCEPT_LOGD_N_OUT(gLogger, "Service is already not running.");
				return TRUE;
			}
			else {
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to stop service. Error: {}", err);
			}
		}
		else {
			INTERCEPT_LOGD_N_OUT(gLogger, "Service stopped successfully.");
		}
		return status;
	}

	BOOL create_or_update_service(const std::vector<std::string>& exe_args) {
		assert(!BIN_PATH.empty() && "Binary path cannot be empty");
		INTERCEPT_LOGD_N_OUTW(gLogger, "Using executable path: {}", BIN_PATH);

		SC_HANDLE scm = get_service_manager();
		if (!scm) {
			return FALSE;
		}

		std::wstring args;
		if (!exe_args.empty()) {
			for (const auto& arg : exe_args) {
				args += fmt::format(LR"("{}" )", utils::string::ConvertUtf8ToWide(arg));
			}
			utils::string::trim(args);
		}

		args = fmt::format(LR"("{}" {})", BIN_PATH, args);
		INTERCEPT_LOGD_N_OUTW(gLogger, "{}", args);

		SC_HANDLE service = CreateServiceW(
			scm,
			SERVICE_NAME,
			DISPLAY_NAME,
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			args.c_str(),
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
		);

		DWORD lastError = GetLastError();
		if (!service && lastError == ERROR_SERVICE_EXISTS) {
			INTERCEPT_LOGD_N_OUT(gLogger, "Service already exists.");

			INTERCEPT_PRINTLN("------------------------------------------------------------------------------------------------------------");
			INTERCEPT_PRINTLN("                                                 SERVICE INFO                                               ");
			INTERCEPT_PRINTLN("------------------------------------------------------------------------------------------------------------");

			run_command(L"sc qdescription " + std::wstring(SERVICE_NAME));
			run_command(L"sc qc " + std::wstring(SERVICE_NAME));

			INTERCEPT_PRINTLN("------------------------------------------------------------------------------------------------------------");
			INTERCEPT_PRINTLN("                                                 xxxxxxxxxxxx                                               ");
			INTERCEPT_PRINTLN("------------------------------------------------------------------------------------------------------------");

			if (!ask_user_to_update()) {
				return FALSE;
			}

			service = get_service();
			if (!service) {
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to open existing service. Error: {}", GetLastError());
				return FALSE;
			}

			// Update the binary path
			if (!ChangeServiceConfigW(
				service,
				SERVICE_NO_CHANGE,
				SERVICE_NO_CHANGE,
				SERVICE_NO_CHANGE,
				args.c_str(),
				nullptr, nullptr, nullptr, nullptr, nullptr,
				DISPLAY_NAME
			)) {
				INTERCEPT_LOGE_N_ERR(gLogger, "Failed to update service config. Error: {}", GetLastError());
			}
		}
		else if (!service) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to create service. Error: {}", lastError);
			return FALSE;
		}
		else {
			INTERCEPT_LOGD_N_OUT(gLogger, "Service created successfully.");
		}

		SERVICE_DESCRIPTION desc = { 0 };
		desc.lpDescription = const_cast<LPWSTR>(DESCRIPTION);
		if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &desc)) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to set description. Error: {}", GetLastError());
		}

		/*
		* Set recovery actions:
		*   - restart on 1st, 2nd, and subsequent failures
		*   - 60000ms (1 minute) delay before restart
		*   - reset failure count after 0 days
		*/
		constexpr DWORD NO_OF_ACTIONS = 3;
		SERVICE_FAILURE_ACTIONS actions = {};
		SC_ACTION scActions[NO_OF_ACTIONS] = {};
		for (auto& action : scActions) {
			action.Type = SC_ACTION_RESTART;
			action.Delay = 60000;
		}
		actions.cActions = NO_OF_ACTIONS;
		actions.lpsaActions = scActions;
		actions.dwResetPeriod = 0;

		if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS, &actions)) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to set recovery actions. Error: {}", GetLastError());
		}
		else {
			INTERCEPT_LOGD_N_OUT(gLogger, "Recovery actions set successfully.");
		}

		// Enable recovery actions on crash
		DWORD crash_recovery = TRUE;
		if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_FAILURE_ACTIONS_FLAG, &crash_recovery)) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to enable recovery. Error: {}", GetLastError());
		}

		return TRUE;
	}

	BOOL delete_service() {
		SC_HANDLE service = get_service();
		assert(service != nullptr && "Service handle cannot be null");
		if (!service) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Service not found! Failed to delete service.");
			return FALSE;
		}
		BOOL status = DeleteService(service);
		if (!status) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to delete service. Error: {}", GetLastError());
			return FALSE;
		}

		INTERCEPT_LOGD_N_OUT(gLogger, "Service deleted successfully.");
		return TRUE;
	}

	DWORD get_service_status() {
		SC_HANDLE service = get_service();
		if (!service) {
			INTERCEPT_LOGE_N_ERR(gLogger, "Service not found! Failed to get service status.");
			return ERROR_NOT_FOUND;
		}
		if (!QueryServiceStatus(service, &g_ServiceStatus)) {
			DWORD err = GetLastError();
			INTERCEPT_LOGE_N_ERR(gLogger, "Failed to query service status. Error: {}", err);
			return err;
		}
		INTERCEPT_LOGE_N_ERR(gLogger, "Service Status: {}", g_ServiceStatus.dwCurrentState);
		return ERROR_SUCCESS;
	}
};
