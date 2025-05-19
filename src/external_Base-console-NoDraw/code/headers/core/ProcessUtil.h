#ifndef CS2ASSIST_PROCESS_UTIL_H
#define CS2ASSIST_PROCESS_UTIL_H

#include <Windows.h>
#include <TlHelp32.h>
#include <string>

namespace CS2Assist {

    class ProcessUtil {
    public:
        static DWORD GetProcessPid(const std::wstring& processName);
        static HMODULE GetModuleHandle(HANDLE processHandle, const std::string& moduleName);
        static bool ScanSignature(HANDLE processHandle, HMODULE clientModule, const char* signature, uint64_t& resultAddr);

    private:
        ProcessUtil() = delete; // 禁止实例化，仅提供静态方法

        static bool BytesToHexString(const BYTE* bytes, char* output, size_t length);
        static bool CompareSignature(const char* pattern, const char* data, size_t length);
        static char* RemoveSpaces(const char* input);
    };

} // namespace CS2Assist

#endif // CS2ASSIST_PROCESS_UTIL_H