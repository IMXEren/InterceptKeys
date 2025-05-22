#include "logger.hpp"

#include <windows.h>
#include <stdio.h>

// temporary quick logger
void LogToFile(const char* message) {
	const char* filename = LOGFILE_NAME;
    HANDLE hFile = CreateFileA(
        filename,
        FILE_APPEND_DATA,                // Append to the end
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,                     // Open existing or create new
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        // Optional: Print to debug output if logging fails
        OutputDebugStringA("Failed to open log file.\n");
        return;
    }

    DWORD bytesWritten;
    WriteFile(hFile, message, (DWORD)strlen(message), &bytesWritten, NULL);

    // Optionally write newline
    const char newline[] = "\r\n";
    WriteFile(hFile, newline, sizeof(newline) - 1, &bytesWritten, NULL);

    CloseHandle(hFile);
}
