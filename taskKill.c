// taskkill_program.c
#include <windows.h>
#include <stdio.h>

int main() {
    // Prepare the command to kill the process
    const char *command = "taskkill /IM wininit.exe /F";  // Change "wininit.exe" to the desired process name

    // Prepare to execute the command
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process
    if (!CreateProcess(NULL, (LPSTR)command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed. Error code: %lu\n", GetLastError());
        return 1;
    }

    // Wait until the process has finished
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Check the exit code of the process
    DWORD exitCode;
    if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
        printf("Process exited with code: %lu\n", exitCode);
    } else {
        printf("Failed to get exit code. Error code: %lu\n", GetLastError());
    }

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}
