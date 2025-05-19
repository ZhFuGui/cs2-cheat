#include "../headers/Utility.h"
#include <Windows.h>
#include <iostream>
#include <thread>

// 显示倒计时暂停
void Utility::VisualPause(int seconds) noexcept {
    std::cout << seconds << " 秒后继续..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    for (int i = seconds; i >= 0; --i) {
        system("cls");
        std::cout << i << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    system("cls");
}

// 毫秒级暂停
void Utility::PauseMs(int milliseconds) noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// 终止当前进程
void Utility::TerminateCurrentProcess() noexcept {
    HANDLE process = GetCurrentProcess();
    if (process) {
        TerminateProcess(process, 0);
        CloseHandle(process);
    }
}

// 激活音效
void Utility::PlayActivationSound() noexcept {
    Beep(1700, 600);
}

// 禁用音效
void Utility::PlayDeactivationSound() noexcept {
    Beep(300, 700);
}