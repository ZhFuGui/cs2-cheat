#include "../headers/XrayController.h"
#include "../headers/FileHandler.h"
#include "../headers/MemoryManager.h"
#include "../headers/PatternScanner.h"
#include "../headers/Utility.h"
#include <iostream>
#include <chrono>
#include <thread>

// ȫ�ֱ���
static uint64_t g_offset = 0;
static uint64_t g_tempOffset = 0;
static int g_xrayState = 3; // 0: �ر�, 1: ����, 3: δ��ʼ�� 

// ��ȡ CS2 Xray ƫ�Ƶ�ַ
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

// ��ʾ�˵�
bool XrayController::DisplayMenu() {
    system("cls");
    std::cout << "\033[6m\033[?25lF1 ���� X-ray\nF2 ���� X-ray\nF3 ���������ļ�\n\033[32mDEL ж�س���\033[0m" << std::endl;
    return true;
}

// ����ƫ�Ƶ�ַ���߳�
DWORD WINAPI XrayController::UpdateOffset(LPVOID param) {
    int id = *static_cast<int*>(param);
    if (id != 0) {
        std::cout << "��ַ�����߳������У�ID: " << id << std::endl;
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

        //������߿��Գ�����std::stringƴ�ӻ�strncpy_s�ȸ��ִ�����ȫ�ķ���
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
    std::cout << "���Ժ����ڸ��»�ַ..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    // ����ƫ�Ƹ����߳�
    int* threadId = new int(1);
    CreateThread(nullptr, 0, XrayController::UpdateOffset, threadId, 0, nullptr);

    // �״θ��»�ַ
    int* initialId = new int(0);
    XrayController::UpdateOffset(initialId);
    delete initialId;

    // ����ַ�Ƿ��ҵ�
    if (g_offset == 0) {
        std::cout << "δ�ҵ���ַ�������а��ս̳�Ѱ��������...\n�������: " << GetLastError() << std::endl;
    }
    else {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "��ַ���ҵ�����ʱ: " << duration.count() << " ����\n�ȴ����븨��..." << std::endl;
        Utility::PauseMs(2000);
    }

    XrayController::DisplayMenu();

    // ��ѭ�������������
    while (true) {
        if (GetAsyncKeyState(VK_F1) & 0x8000) { // F1 ���� Xray
            if (g_offset == 0) {
                Utility::PlayDeactivationSound();
                continue;
            }
            if (MemoryManager::WriteMemory(L"cs2.exe", L"client.dll", g_offset, 0x9090, 2)) {
                g_xrayState = 1;
                Utility::PlayActivationSound();
            }
            else {
                std::cout << "�������: " << GetLastError() << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_F2) & 0x8000) { // F2 ���� Xray
            if (g_offset == 0) {
                Utility::PlayDeactivationSound();
                continue;
            }
            if (MemoryManager::WriteMemory(L"cs2.exe", L"client.dll", g_offset, 0xC032, 2)) {
                g_xrayState = 0;
                Utility::PlayDeactivationSound();
            }
            else {
                std::cout << "�������: " << GetLastError() << std::endl;
            }
        }

        if (GetAsyncKeyState(VK_F3) & 0x8000) { // F3 �������ļ�
            char* userProfile = nullptr;
            size_t len = 0;
            if (_dupenv_s(&userProfile, &len, "USERPROFILE") != 0 || !userProfile) {
                MessageBoxW(nullptr, L"�޷���ȡ�û������ļ�·����", L"����", 0);
                return -1;
            }
            std::string filePath = std::string(userProfile) + "\\Documents\\cs2Xray.ini";
            free(userProfile);
            FileHandler::OpenTextFile(filePath);
        }

        if (GetAsyncKeyState(VK_DELETE) & 0x8000) { // DEL ж�س���
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