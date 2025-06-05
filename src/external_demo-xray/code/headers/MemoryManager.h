#pragma once
#include <Windows.h>
#include <string>

// ������ö��
enum class ErrorCode {
    Success = 1,
    ProcessNotFound,
    ProcessOpenFailed,
    ModuleNotFound,
    MemoryWriteFailed
};

// �ڴ�����࣬������̺��ڴ����
class MemoryManager {
public:

    // ��ȡ���̾��
    static HANDLE GetProcessHandle(const std::wstring& processName);

    // ��ȡģ����
    static HMODULE GetModuleHandle(HANDLE process, const std::wstring& moduleName);

    // д���ڴ�
    static bool WriteMemory(const std::wstring& processName, const std::wstring& moduleName,
        uint64_t offset, uint64_t value, DWORD length);
};