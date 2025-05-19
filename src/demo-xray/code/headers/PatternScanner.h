#pragma once
#include <Windows.h>
#include <iostream>

// 特征码扫描类，负责在进程内存中查找我们需要的偏移
class PatternScanner {
public:

    // 将字节数组转换为字符串
    static bool BytesToString(BYTE* byteCode, char* strCode, int codeLen);

    // 比较字符串是否匹配
    static bool CompareString(const char* pattern, const char* readStr, int cmpLen);

    // 扫描游戏内存中的特征码
    static bool ScanMemory(HANDLE process, uint64_t startAddr, uint64_t endAddr,
        const char* pattern, int patternLen, uint64_t& resultAddr);
};