#include "../headers/XrayController.h"
#include "../headers/FileHandler.h"
#include "../headers/MemoryManager.h"
#include "../headers/PatternScanner.h"
#include "../headers/Utility.h"
#include <iostream>
#include <chrono>
#include <thread>

// 全局变量
static uint64_t g_offset = 0;
static uint64_t g_tempOffset = 0;
static int g_xrayState = 3; // 0: 关闭, 1: 启用, 3: 未初始化 

// 获取 CS2 Xray 偏移地址
bool XrayController::GetXrayOffset(uint64_t& offset, const char* signature) {

    HANDLE process = MemoryManager::GetProcessHandle(L"cs2.exe");
    if (!process) {
        return false;
    }

    HMODULE clientModule = MemoryManager::GetModuleHandle(process, L"client.dll");
    if (!clientModule) {
        CloseHandle(process);
        return false;
    }

    uint64_t addr = 0;
    if (!PatternScanner::ScanMemory(process, reinterpret_cast<uint64_t>(clientModule),
        reinterpret_cast<uint64_t>(clientModule) + 0x1000000,
        signature, static_cast<int>(strlen(signature)), addr)) {
        offset = 0;
        CloseHandle(process);
        return false;
    }

    offset = addr - reinterpret_cast<uint64_t>(clientModule);
    CloseHandle(process);
    return true;
}

// 显示菜单
bool XrayController::DisplayMenu() {
    system("cls");
    std::cout << "\033[6m\033[?25lF1 启用 X-ray\nF2 禁用 X-ray\nF3 打开特征码文件\n\033[32mDEL 卸载程序\033[0m" << std::endl;
    return true;
}

// 更新偏移地址的线程
DWORD WINAPI XrayController::UpdateOffset(LPVOID param) {
    int id = *static_cast<int*>(param);
    if (id != 0) {
        std::cout << "基址更新线程运行中！ID: " << id << std::endl;
    }

    do {
        char* signature = FileHandler::ReadSignature();
        if (!signature) {
            Utility::PauseMs(200);
            continue;
        }

        if (GetXrayOffset(g_tempOffset, signature)) {
            g_offset = g_tempOffset;
            g_xrayState = 0;
            delete[] signature;
            Utility::PauseMs(200);
            continue;
        }

        char* altSignature = new char[strlen(signature) + 1];
        strcpy_s(altSignature, strlen(signature) + 1, signature);

        //这里读者可以尝试用std::string拼接或strncpy_s等更现代更安全的方法
        altSignature[0] = '9';
        altSignature[1] = '0';
        altSignature[2] = '9';
        altSignature[3] = '0';

        if (GetXrayOffset(g_tempOffset, altSignature)) {
            g_offset = g_tempOffset;
            g_xrayState = 1;
            delete[] signature;
            delete[] altSignature;
            Utility::PauseMs(200);
            continue;
        }

        g_offset = 0;
        g_xrayState = 0;
        delete[] signature;
        delete[] altSignature;
        Utility::PauseMs(200);
    } while (id);

    return 0;
}

int main() {
    std::cout << "请稍候！正在更新基址..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // 启动偏移更新线程
    int* threadId = new int(1);
    CreateThread(nullptr, 0, XrayController::UpdateOffset, threadId, 0, nullptr);

    // 首次更新基址
    int* initialId = new int(0);
    XrayController::UpdateOffset(initialId);
    delete initialId;

    // 检查基址是否找到
    if (g_offset == 0) {
        std::cout << "未找到基址！请自行按照教程寻找特征码...\n错误代码: " << GetLastError() << std::endl;
    }
    else {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "基址已找到！耗时: " << duration.count() << " 毫秒\n等待进入辅助..." << std::endl;
        Utility::PauseMs(2000);
    }

    XrayController::DisplayMenu();

    // 主循环处理键盘输入
    while (true) {
        if (GetAsyncKeyState(VK_F1) & 0x8000) { // F1 启用 Xray
            if (g_offset == 0) {
                Utility::PlayDeactivationSound();
                continue;
            }
            if (MemoryManager::WriteMemory(L"cs2.exe", L"client.dll", g_offset, 0x9090, 2)) {
                g_xrayState = 1;
                Utility::PlayActivationSound();
            }
            else {
                std::cout << "错误代码: " << GetLastError() << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_F2) & 0x8000) { // F2 禁用 Xray
            if (g_offset == 0) {
                Utility::PlayDeactivationSound();
                continue;
            }
            if (MemoryManager::WriteMemory(L"cs2.exe", L"client.dll", g_offset, 0xC032, 2)) {
                g_xrayState = 0;
                Utility::PlayDeactivationSound();
            }
            else {
                std::cout << "错误代码: " << GetLastError() << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_F3) & 0x8000) { // F3 打开配置文件
            char* userProfile = nullptr;
            size_t len = 0;
            if (_dupenv_s(&userProfile, &len, "USERPROFILE") != 0 || !userProfile) {
                MessageBoxW(nullptr, L"无法获取用户配置文件路径！", L"错误", 0);
                return -1;
            }
            std::string filePath = std::string(userProfile) + "\\Documents\\cs2Xray.ini";
            free(userProfile);
            FileHandler::OpenTextFile(filePath);
        }

        if (GetAsyncKeyState(VK_DELETE) & 0x8000) { // DEL 卸载程序
            if (g_offset != 0) {
                MemoryManager::WriteMemory(L"cs2.exe", L"client.dll", g_offset, 0xC032, 2);
                g_xrayState = 0;
                Utility::PlayDeactivationSound();
            }
            Utility::TerminateCurrentProcess();
        }

        Utility::PauseMs(10);
    }

    delete threadId;
    return 0;
}