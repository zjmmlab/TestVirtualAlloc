#include "mainwindow.h"

#include <QApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

#include <iostream>

bool IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(
            &ntAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &administratorsGroup)) {
        CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }

    return isAdmin;
}

bool RelaunchAsAdmin() {
    // Get the file path of the current program
    wchar_t filePath[MAX_PATH];
    GetModuleFileName(NULL, filePath, MAX_PATH);

    // Set up the SHELLEXECUTEINFO structure
    SHELLEXECUTEINFOW sei = { sizeof(SHELLEXECUTEINFOW) };
    sei.lpVerb = L"runas"; // Requesting Administrator Rights
    sei.lpFile = filePath; // Current program path
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&sei)) {
        DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) {
            MessageBox(NULL, L"Privilege escalation canceled by user!", L"Tip", MB_ICONEXCLAMATION);
        } else {
            MessageBox(NULL, L"Unable to escalate privileges!", L"Error", MB_ICONERROR);
        }

        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN

    if (!IsRunningAsAdmin()) {
        std::cout << "The current program does not have administrator privileges, try restarting...\n";
        if (!RelaunchAsAdmin()) return 0;
        std::cout << "The program is running with administrator privileges!\n";

        // Get the current process handle
        HANDLE hProcess = GetCurrentProcess();
        SIZE_T minSize, maxSize;

        // Get the working set size of the current process
        if (GetProcessWorkingSetSize(hProcess, &minSize, &maxSize)) {
            std::cout << "Current Working Set Size:" << std::endl;
            std::cout << "Minimum: " << minSize / (1024 * 1024) << "MB" << std::endl;
            std::cout << "maxSize: " << maxSize / (1024 * 1024) << "MB" << std::endl;
        } else {
            std::cout << "Failed to get working set size. Error: " << GetLastError() << std::endl;
        }

        // Modify the working set size of the current process
        SIZE_T newMinSize = 2.5 * 1024 * 1024 * 1024; // 2.5 GB
        SIZE_T newMaxSize = 2.5 * 1024 * 1024 * 1024; // 2.5 GB
        if (SetProcessWorkingSetSize(hProcess, newMinSize, newMaxSize)) {
            std::cout << "Working set size updated successfully." << std::endl;
            std::cout << "New Minimum: " << newMinSize / (1024 * 1024) << "MB" << std::endl;
            std::cout << "New Maximum: " << newMaxSize / (1024 * 1024) << "MB" << std::endl;
        } else {
            std::cout << "Failed to set working set size. Error: " << GetLastError() << std::endl;
        }

        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();
    }

#endif

    //QApplication a(argc, argv);
    //MainWindow w;
    //w.show();

    return 0;
}

