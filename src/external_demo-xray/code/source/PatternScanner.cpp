#include "../headers/PatternScanner.h"

// 将字节数组转换为字符串
bool PatternScanner::BytesToString(BYTE* byteCode, char* strCode, int codeLen) {
    for (int i = 0; i < codeLen; ++i) {
        wsprintfA(&strCode[i * 2], "%02X", byteCode[i]);
    }
    return true;
}

// 比较字符串是否匹配
bool PatternScanner::CompareString(const char* pattern, const char* readStr, int cmpLen) {
    for (int i = 0; i < cmpLen; ++i) {
        if (pattern[i] == '?') {
            continue;
        }
        if (pattern[i] != readStr[i]) {
            return false;
        }
    }
    return true;
}

// 扫描游戏内存中的特征码
bool PatternScanner::ScanMemory(HANDLE process, uint64_t startAddr, uint64_t endAddr,
    const char* pattern, int patternLen, uint64_t& resultAddr) {
    BYTE* readCode = new BYTE[0x1000];
    if (!readCode) {
        return false;
    }

    for (uint64_t addr = startAddr; addr <= endAddr - 0x1000; addr += 0x1000 - patternLen) {
        char strCode[0x2001] = { 0 };
        if (!ReadProcessMemory(process, reinterpret_cast<LPVOID>(addr), readCode, 0x1000, nullptr)) {
            continue;
        }

        BytesToString(readCode, strCode, 0x1000);
        for (int i = 0; i < 0x2001 - patternLen; ++i) {
            if (CompareString(pattern, &strCode[i], patternLen)) {
                resultAddr = addr + i / 2;
                delete[] readCode;
                return true;
            }
        }
    }

    delete[] readCode;
    return false;
}