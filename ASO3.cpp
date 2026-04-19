#include <windows.h>
#include <iostream>
#include <vector>
#include <winsvc.h>
#include <psapi.h>
#include <clocale>


#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")

// Tema optionala: identificare DDL
void ListServiceModules(DWORD processID)
{
    
    if (processID == 0) return;

    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (NULL == hProcess) return;

    HMODULE hMods[1024];
    DWORD cbNeeded;

    
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t)))
            {
                std::wcout << L"  [DLL]: " << szModName << std::endl;
            }
        }
    }
    CloseHandle(hProcess);
}

// TEMA OBLIGATORIE: Identificarea serviciilor active
void ListRunningServices()
{
    std::wcout << L"Se interogheaza Managerul de Servicii (SCM)..." << std::endl;

    
    SC_HANDLE hScm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);
    if (hScm == NULL) {
        std::wcerr << L"EROARE: Nu se poate accesa SCM. Ruleaza ca ADMINISTRATOR! (Cod: " << GetLastError() << L")" << std::endl;
        return;
    }

    DWORD dwBytesNeeded = 0, dwServicesReturned = 0, dwResumeHandle = 0;

    
    EnumServicesStatusExW(
        hScm,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL, 
        NULL, 0, &dwBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL);

    if (dwBytesNeeded == 0) {
        std::wcout << L"Nu s-au gasit date despre servicii." << std::endl;
        CloseServiceHandle(hScm);
        return;
    }

    
    LPENUM_SERVICE_STATUS_PROCESSW pServices = (LPENUM_SERVICE_STATUS_PROCESSW)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

    if (pServices && EnumServicesStatusExW(
        hScm,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        (LPBYTE)pServices, dwBytesNeeded, &dwBytesNeeded, &dwServicesReturned, &dwResumeHandle, NULL))
    {
        int count = 0;
        for (DWORD i = 0; i < dwServicesReturned; i++)
        {
            
            if (pServices[i].ServiceStatusProcess.dwCurrentState == SERVICE_RUNNING) {
                std::wcout << L"\n>>> SERVICIU ACTIV: " << pServices[i].lpDisplayName
                    << L"\n    PID: " << pServices[i].ServiceStatusProcess.dwProcessId << std::endl;

                
                ListServiceModules(pServices[i].ServiceStatusProcess.dwProcessId);

                std::wcout << L"--------------------------------------------------" << std::endl;
                count++;
            }
        }
        std::wcout << L"\nFinalizat! S-au gasit " << count << L" servicii active." << std::endl;
    }
    else {
        std::wcerr << L"Eroare la popularea listei: " << GetLastError() << std::endl;
    }

    if (pServices) HeapFree(GetProcessHeap(), 0, pServices);
    CloseServiceHandle(hScm);
}

int wmain()
{
    
    std::setlocale(LC_ALL, "");

    ListRunningServices();

    std::wcout << L"\nApasati ENTER pentru a inchide..." << std::endl;
    std::wcin.get();
    return 0;
}