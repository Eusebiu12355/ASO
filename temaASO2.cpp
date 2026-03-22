#include <windows.h>
#include <iostream>
#include <string>

HKEY GetRootKey(const std::wstring& wstrRoot)
{
    if (wstrRoot == L"HKCU")
        return HKEY_CURRENT_USER;
    else if (wstrRoot == L"HKLM")
        return HKEY_LOCAL_MACHINE;
    else
        return NULL;
}

bool OpenRegistryKey(HKEY hRootKey, const std::wstring& wstrSubKey, HKEY& hOpenedKey)
{
    LONG lResult = RegOpenKeyExW(
        hRootKey,
        wstrSubKey.c_str(),
        0,
        KEY_READ,
        &hOpenedKey
    );

    return (lResult == ERROR_SUCCESS);
}

void PrintSubKeys(HKEY hKey)
{
    DWORD dwIndex = 0;
    wchar_t wszName[256];
    DWORD dwSize = 256;

    while (true)
    {
        dwSize = 256;

        LONG lResult = RegEnumKeyExW(
            hKey,
            dwIndex,
            wszName,
            &dwSize,
            NULL,
            NULL,
            NULL,
            NULL
        );

        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

        if (lResult == ERROR_SUCCESS)
        {
            std::wcout << L"Subcheie [" << dwIndex << L"]: " << wszName << std::endl;
        }
        else
        {
            std::wcerr << L"Eroare la citire." << std::endl;
            break;
        }

        dwIndex++;
    }

    std::wcout << L"\nTotal subchei: " << dwIndex << std::endl;
}

int wmain()
{
    std::wstring wstrRoot;
    std::wstring wstrSubKey;

    std::wcout << L"Introdu root key (HKCU / HKLM): ";
    std::getline(std::wcin, wstrRoot);

    std::wcout << L"Introdu subkey (ex: Software): ";
    std::getline(std::wcin, wstrSubKey);

    HKEY hRootKey = GetRootKey(wstrRoot);

    if (hRootKey == NULL)
    {
        std::wcerr << L"Root key invalid!" << std::endl;
        return 1;
    }

    HKEY hOpenedKey;

    if (!OpenRegistryKey(hRootKey, wstrSubKey, hOpenedKey))
    {
        std::wcerr << L"Nu s-a putut deschide cheia!" << std::endl;
        return 1;
    }

    std::wcout << L"\nSubcheile sunt:\n" << std::endl;

    PrintSubKeys(hOpenedKey);

    RegCloseKey(hOpenedKey);

    return 0;
}