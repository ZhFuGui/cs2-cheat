#include "../../headers/core/ProcessUtil.h"
#include <psapi.h>
#include <cstring>
#include <memory>

namespace CS2Assist {

    DWORD ProcessUtil::GetProcessPid(const std::wstring& processName) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 0;

        PROCESSENTRY32W entry{ sizeof(PROCESSENTRY32W) };
        if (!Process32FirstW(snapshot, &entry)) {
            CloseHandle(snapshot);
            return 0;
        }

        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));

        CloseHandle(snapshot);
        return 0;
    }

    HMODULE ProcessUtil::GetModuleHandle(HANDLE processHandle, const std::string& moduleName) {
        HMODULE modules[999];
        DWORD bytesNeeded;

        if (!EnumProcessModulesEx(processHandle, modules, sizeof(modules), &bytesNeeded, LIST_MODULES_ALL)) {
            return nullptr;
        }

        char baseName[100];
        for (size_t i = 0, count = bytesNeeded / sizeof(HMODULE); i < count; ++i) {
            if (GetModuleBaseNameA(processHandle, modules[i], baseName, sizeof(baseName)) > 0 &&
                _stricmp(baseName, moduleName.c_str()) == 0) {
                return modules[i];
            }
        }
        return nullptr;
    }

    bool ProcessUtil::ScanSignature(HANDLE processHandle, HMODULE clientModule, const char* signature, uint64_t& resultAddr) {
        MODULEINFO modInfo;
        if (!K32GetModuleInformation(processHandle, clientModule, &modInfo, sizeof(modInfo))) {
            return false;
        }

        uint64_t startAddr = reinterpret_cast<uint64_t>(clientModule);
        uint64_t endAddr = startAddr + modInfo.SizeOfImage;
        std::unique_ptr<char[]> noSpaceSig(RemoveSpaces(signature));
        if (!noSpaceSig) return false;

        size_t sigLength = strlen(noSpaceSig.get());
        if (sigLength == 0) return false;

        constexpr size_t BUFFER_SIZE = 0x1000;
        std::unique_ptr<BYTE[]> readBuffer(new BYTE[BUFFER_SIZE]);
        std::unique_ptr<char[]> hexBuffer(new char[BUFFER_SIZE * 2 + 1]);

        for (uint64_t addr = startAddr; addr <= endAddr - BUFFER_SIZE; addr += BUFFER_SIZE) {
            SIZE_T bytesRead;
            if (!ReadProcessMemory(processHandle, reinterpret_cast<LPCVOID>(addr), readBuffer.get(), BUFFER_SIZE, &bytesRead) ||
                bytesRead < sigLength / 2) {
                continue;
            }

            std::memset(hexBuffer.get(), 0, BUFFER_SIZE * 2 + 1);
            if (!BytesToHexString(readBuffer.get(), hexBuffer.get(), bytesRead)) continue;

            for (size_t i = 0; i <= bytesRead * 2 - sigLength; i += 2) {
                if (CompareSignature(noSpaceSig.get(), &hexBuffer[i], sigLength)) {
                    resultAddr = addr + i / 2;
                    return true;
                }
            }
        }
        return false;
    }

    bool ProcessUtil::BytesToHexString(const BYTE* bytes, char* output, size_t length) {
        if (!bytes || !output || length == 0) return false;

        for (size_t i = 0; i < length; ++i) {
            wsprintfA(&output[i * 2], "%02X", bytes[i]);
        }
        return true;
    }

    bool ProcessUtil::CompareSignature(const char* pattern, const char* data, size_t length) {
        if (!pattern || !data || length == 0) return false;

        for (size_t i = 0; i < length; ++i) {
            if (pattern[i] != '?' && pattern[i] != data[i]) {
                return false;
            }
        }
        return true;
    }

    char* ProcessUtil::RemoveSpaces(const char* input) {
        if (!input) return nullptr;

        size_t len = strlen(input);
        size_t newLen = 0;
        for (size_t i = 0; i < len; ++i) {
            if (input[i] != ' ') ++newLen;
        }

        char* result = new char[newLen + 1];
        if (!result) return nullptr;

        for (size_t i = 0, j = 0; i < len; ++i) {
            if (input[i] != ' ') result[j++] = input[i];
        }
        result[newLen] = '\0';
        return result;
    }

} // namespace CS2Assist