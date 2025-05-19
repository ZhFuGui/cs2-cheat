#pragma once
#include <chrono>

// 实用工具类，提供暂停、声音和进程终止功能
class Utility {
public:
    // 显示倒计时暂停
    static void VisualPause(int seconds) noexcept;

    // 毫秒级暂停
    static void PauseMs(int milliseconds) noexcept;

    // 终止当前进程
    static void TerminateCurrentProcess() noexcept;

    // 激活音效
    static void PlayActivationSound() noexcept;

    // 禁用音效
    static void PlayDeactivationSound() noexcept;
};