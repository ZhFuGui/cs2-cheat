#pragma once
#include <Windows.h>
#include <iostream>

// ������ɨ���࣬�����ڽ����ڴ��в���������Ҫ��ƫ��
class PatternScanner {
public:

    // ���ֽ�����ת��Ϊ�ַ���
    static bool BytesToString(BYTE* byteCode, char* strCode, int codeLen);

    // �Ƚ��ַ����Ƿ�ƥ��
    static bool CompareString(const char* pattern, const char* readStr, int cmpLen);

    // ɨ����Ϸ�ڴ��е�������
    static bool ScanMemory(HANDLE process, uint64_t startAddr, uint64_t endAddr,
        const char* pattern, int patternLen, uint64_t& resultAddr);
};