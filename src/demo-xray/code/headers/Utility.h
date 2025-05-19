#pragma once
#include <chrono>

// ʵ�ù����࣬�ṩ��ͣ�������ͽ�����ֹ����
class Utility {
public:
    // ��ʾ����ʱ��ͣ
    static void VisualPause(int seconds) noexcept;

    // ���뼶��ͣ
    static void PauseMs(int milliseconds) noexcept;

    // ��ֹ��ǰ����
    static void TerminateCurrentProcess() noexcept;

    // ������Ч
    static void PlayActivationSound() noexcept;

    // ������Ч
    static void PlayDeactivationSound() noexcept;
};