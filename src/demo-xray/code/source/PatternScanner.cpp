#include "../headers/PatternScanner.h"

// ���ֽ�����ת��Ϊ�ַ���
bool PatternScanner::BytesToString(BYTE* byteCode, char* strCode, int codeLen) {
    for (int i = 0; i < codeLen; ++i) {
        wsprintfA(&strCode[i * 2], "%02X", byteCode[i]);
    }
    return true;
}

// �Ƚ��ַ����Ƿ�ƥ��
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

// ɨ����Ϸ�ڴ��е�������
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