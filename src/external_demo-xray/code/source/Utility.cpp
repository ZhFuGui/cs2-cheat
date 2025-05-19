#include "../headers/Utility.h"
#include <Windows.h>
#include <iostream>
#include <thread>

// ��ʾ����ʱ��ͣ
void Utility::VisualPause(int seconds) noexcept {
    std::cout << seconds << " ������..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (int i = seconds; i >= 0; --i) {
        system("cls");
        std::cout << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    system("cls");
}

// ���뼶��ͣ
void Utility::PauseMs(int milliseconds) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// ��ֹ��ǰ����
void Utility::TerminateCurrentProcess() noexcept {
    HANDLE process = GetCurrentProcess();
    if (process) {
        TerminateProcess(process, 0);
        CloseHandle(process);
    }
}

// ������Ч
void Utility::PlayActivationSound() noexcept {
    Beep(1700, 600);
}

// ������Ч
void Utility::PlayDeactivationSound() noexcept {
    Beep(300, 700);
}