// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // 当 DLL 第一次被加载到进程地址空间时调用
        break;

    case DLL_THREAD_ATTACH:
        // 当进程中创建新线程时调用（不是每个线程都会触发）
        break;

    case DLL_THREAD_DETACH:
        // 当线程退出时调用（可能不会触发，如果进程已终止）
        break;

    case DLL_PROCESS_DETACH:
        // 当 DLL 被卸载时调用
        break;
    }

    return TRUE; // 返回 FALSE 表示失败，此时系统不会继续加载 DLL
}