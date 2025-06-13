
#include "Service.hpp"

#include "common.h"
#include "logger.hpp"

#include <sstream>
#include <string>
#include <string_view>

SERVICE_STATUS g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE g_ServiceStopEvent = nullptr;

DWORD getGlobalMutex(HANDLE* hMutex) {
	*hMutex = CreateMutexA(nullptr, FALSE, "Global\\InterceptKeysMutex");
	if (*hMutex == nullptr) {
		return 1;
	}
	return GetLastError();
}

void ConvertCommandLineArgs(DWORD dwArgc, LPTSTR* lpszArgv, int* argcOut, char*** argvOut) {
	int argc = (int)dwArgc;
	char** argv = (char**)malloc(sizeof(char*) * argc);
	if (argv == nullptr) {
		// Handle memory allocation failure
		*argcOut = 0;
		*argvOut = nullptr;
		return;
	}

	for (int i = 0; i < argc; ++i) {
#ifdef UNICODE
		// Convert from wchar_t* to char*
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, lpszArgv[i], -1, NULL, 0, NULL, NULL);
		argv[i] = (char*)malloc(size_needed);
		WideCharToMultiByte(CP_UTF8, 0, lpszArgv[i], -1, argv[i], size_needed, NULL, NULL);
#else
		// Direct cast if not UNICODE
		argv[i] = _strdup(lpszArgv[i]);
#endif
	}

	*argcOut = argc;
	*argvOut = argv;
}

int AppMain(int argc, char* argv[]) {
	HANDLE hMutex;
	int err;

	DEBUG_PRINT("Trying to get a global mutex...\n");
	LOG_DEBUG(gLogger, "Trying to get a global mutex...");
	if ((err = getGlobalMutex(&hMutex)) != 0) {
		std::stringstream msgStream;
		if (err == 1) {
			msgStream << "(nullptr)";
		}
		else if (err != 0) {
			msgStream << std::system_category().message(err) << " error: " << err;
		}
		DEBUG_PRINT("Failed to create mutex: %s\n", msgStream.str().c_str());
		LOG_DEBUG(gLogger,"Failed to create mutex: {}", msgStream.str());
		return err;
	}

	DEBUG_PRINT("Interception has started!\n");
	LOG_DEBUG(gLogger, "Interception has started!");
	int status = InterceptMain(argc, argv);
	DEBUG_PRINT("Interception has stopped! status: %d\n", status);
	LOG_DEBUG(gLogger,"Interception has stopped! status: {}", status);

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return status;
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
	g_StatusHandle = RegisterServiceCtrlHandler(INTERCEPT_SERVICE_NAME, ServiceCtrlHandler);
	if (!g_StatusHandle) return;

	// Set initial status
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Create event to signal stop
	g_ServiceStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!g_ServiceStopEvent) {
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
		return;
	}

	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

	// Application
	ProgramArgs params;
	ConvertCommandLineArgs(argc, argv, &params.argc, &params.argv);
	HANDLE hThread = CreateThread(nullptr, 0, ServiceWorkerThread, &params, 0, nullptr);
	LOG_DEBUG(gLogger, "Service worker thread has started!");
	WaitForSingleObject(g_ServiceStopEvent, INFINITE);
	free(params.argv);

	// When finished or stopped:
	CloseHandle(g_ServiceStopEvent);
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwCheckPoint = 2;
	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
	switch (CtrlCode) {
	case SERVICE_CONTROL_STOP:
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

		SetEvent(g_ServiceStopEvent);
		break;
	default:
		break;
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {
	ProgramArgs params;
	if (lpParam) params = *(ProgramArgs*)lpParam;
	else params = { 0, nullptr };
	DWORD status = (DWORD)AppMain(params.argc, params.argv);
	ServiceCtrlHandler(SERVICE_CONTROL_STOP);
	return status;
}

int main(int argc, char* argv[]) {
	initLogging();
	DEBUG_PRINT("Starting InterceptKeys Service...\n");
	LOG_DEBUG(gLogger, "Starting InterceptKeys Service...");

#ifdef BUILD_AS_SERVICE
	// Run as a service
	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{ (LPWSTR)INTERCEPT_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ nullptr, nullptr }
	};

	if (!StartServiceCtrlDispatcher(ServiceTable)) {
		// Failed to connect to service control manager
		DWORD error = GetLastError();
		std::string message = std::system_category().message(error);
		DEBUG_PRINT("Failed to start service control dispatcher: %s\n", message.c_str());
		LOG_DEBUG(gLogger,"Failed to start service control dispatcher: {}", message);
		return error;
	}
	return 0;
#else
	// Run as a console application
	int status = AppMain(argc, argv);
	return status;
#endif // BUILD_AS_SERVICE
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// These won't be passed to the ServiceMain
	// but still here for completeness
	return main(__argc, __argv);
}