#pragma once
#define NOMINMAX
#include <windows.h>

#define INTERCEPT_SERVICE_NAME L"InterceptKeysService"
extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_StatusHandle;
extern HANDLE g_ServiceStopEvent;

void WINAPI ServiceCtrlHandler(DWORD CtrlCode);
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
int InterceptMain(int argc, char* argv[]);

typedef struct {
	int argc;
	char** argv;
} ProgramArgs;

DWORD getGlobalMutex(HANDLE* hMutex);
void ConvertCommandLineArgs(DWORD dwArgc, LPTSTR* lpszArgv, int* argcOut, char*** argvOut);