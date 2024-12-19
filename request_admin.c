// request_admin.c
#include <windows.h>
#include <stdio.h>

void RunTaskkillProgram() {
    // Prepare to launch the taskkill program
    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof(sei);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.hwnd = NULL;
    sei.lpVerb = "runas";  // Request elevation
    sei.lpFile = "a.exe";  // The name of the taskkill program
    sei.lpParameters = "";  // Add any parameters if needed
    sei.nShow = SW_SHOWNORMAL;

    // Execute with elevated privileges
    if (!ShellExecuteEx(&sei)) {
        printf("Error requesting admin privileges. Error code: %lu\n", GetLastError());
        return;
    }

    // Wait for the elevated process to finish
    WaitForSingleObject(sei.hProcess, INFINITE);
    CloseHandle(sei.hProcess);
}

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
        RunTaskkillProgram();
        return 0;  // Exit the current instance
    } else {
        printf("Running as Administrator. Executing taskkill program...\n");
        RunTaskkillProgram();
    }

    return 0;
}
