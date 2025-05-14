#pragma once
#include <cstdint>
#include <Windows.h>

// Xray 控制器类，管理 Xray 功能的逻辑
class XrayController {
public:

    // 获取 CS2 Xray 偏移地址
    static bool GetXrayOffset(uint64_t& offset, const char* signature);

    // 显示菜单
    static bool DisplayMenu();

    // 更新偏移地址的线程
    static DWORD WINAPI UpdateOffset(LPVOID param);
};