#include "../headers/MemoryManager.h"
#include <TlHelp32.h>
#include <psapi.h>

// 获取进程句柄
HANDLE MemoryManager::GetProcessHandle(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W entry = { sizeof(PROCESSENTRY32W) };
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
                CloseHandle(snapshot);
                return hProcess;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return 0;
}

// 获取模块句柄
HMODULE MemoryManager::GetModuleHandle(HANDLE process, const std::wstring& moduleName) {
    HMODULE modules[1024];
    DWORD needed;
    if (!EnumProcessModulesEx(process, modules, sizeof(modules), &needed, LIST_MODULES_ALL)) {
        return nullptr;
    }

    wchar_t baseName[MAX_PATH];
    for (size_t i = 0; i < needed / sizeof(HMODULE); ++i) {
        if (GetModuleBaseNameW(process, modules[i], baseName, MAX_PATH)) {
            if (_wcsicmp(baseName, moduleName.c_str()) == 0) {
                return modules[i];
            }
        }
    }

    MessageBoxW(nullptr, L"未找到模块句柄！", L"错误", 0);
    return nullptr;
}

// 写入内存
bool MemoryManager::WriteMemory(const std::wstring& processName, const std::wstring& moduleName,
    uint64_t offset, uint64_t value, DWORD length) {

    HANDLE process = GetProcessHandle(processName);
    if (!process) {
        SetLastError(static_cast<DWORD>(ErrorCode::ProcessOpenFailed));
        return false;
    }

    HMODULE moduleAddress = moduleName.empty() ? nullptr : GetModuleHandle(process, moduleName);
    if (!moduleName.empty() && !moduleAddress) {
        CloseHandle(process);
        SetLastError(static_cast<DWORD>(ErrorCode::ModuleNotFound));
        return false;
    }

    bool result = WriteProcessMemory(process, reinterpret_cast<LPVOID>(reinterpret_cast<uint64_t>(moduleAddress) + offset),
        &value, length, nullptr) != 0;

    if (!result) {
        SetLastError(static_cast<DWORD>(ErrorCode::MemoryWriteFailed));
    }

    CloseHandle(process);
    return result;
}