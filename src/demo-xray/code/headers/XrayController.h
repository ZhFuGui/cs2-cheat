#pragma once
#include <cstdint>
#include <Windows.h>

// Xray �������࣬���� Xray ���ܵ��߼�
class XrayController {
public:

    // ��ȡ CS2 Xray ƫ�Ƶ�ַ
    static bool GetXrayOffset(uint64_t& offset, const char* signature);

    // ��ʾ�˵�
    static bool DisplayMenu();

    // ����ƫ�Ƶ�ַ���߳�
    static DWORD WINAPI UpdateOffset(LPVOID param);
};