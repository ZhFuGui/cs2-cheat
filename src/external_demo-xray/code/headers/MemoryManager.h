#pragma once
#include <Windows.h>
#include <string>

// 错误码枚举
enum class ErrorCode {
    Success = 1,
    ProcessNotFound,
    ProcessOpenFailed,
    ModuleNotFound,
    MemoryWriteFailed
};

// 内存管理类，负责进程和内存操作
class MemoryManager {
public:

    // 获取进程句柄
    static HANDLE GetProcessHandle(const std::wstring& processName);

    // 获取模块句柄
    static HMODULE GetModuleHandle(HANDLE process, const std::wstring& moduleName);

    // 写入内存
    static bool WriteMemory(const std::wstring& processName, const std::wstring& moduleName,
        uint64_t offset, uint64_t value, DWORD length);
};