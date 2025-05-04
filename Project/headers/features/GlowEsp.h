#ifndef CS2ASSIST_GLOW_ESP_H
#define CS2ASSIST_GLOW_ESP_H

#include <Windows.h>
#include <atomic>
#include "../entity/EntityMgr.h"
#include "../core/ProcessUtil.h"
#include "../utils/ConstsUtil.h"

namespace CS2Assist {

    enum class GlowMode {
        BlueXray,      // ��ɫ��͸��
        GlowFunction   // ������Ϸ���⺯����͸��
    };

    // ARGB ��ɫ�ṹ��
    struct Color {
        uint8_t a; // Alpha (͸����/����)
        uint8_t r; // Red
        uint8_t g; // Green
        uint8_t b; // Blue

        Color(uint8_t alpha = 255, uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            : a(alpha), r(red), g(green), b(blue) {
        }

        uint32_t ToUInt32() const {
            return (a << 24) | (r << 16) | (g << 8) | b;
        }
    };

    // ȫ�־�̬͸������
    namespace GlowConfig {
        inline std::atomic<bool> isActive{ true };              // ͸���Ƿ���
        inline GlowMode mode{ GlowMode::GlowFunction };         // ��ǰ͸��ģʽ
        inline bool glowTeammates{ false };                     // �Ƿ���ƶ���
        inline bool glowEnemies{ true };                        // �Ƿ���Ƶ���
        inline Color teammateColor{ 255, 0, 255, 0 };           // ������ɫ����ɫ��
        inline Color enemyColor{ 255, 0, 255, 255 };            // ������ɫ����ɫ��
        inline Color targetColor{ 255, 0, 0, 255 };             // Ŀ����ɫ����ɫ��
    };

    class GlowEsp {
    public:
        // ��̬������ֱ��ʹ��ȫ������
        static void ApplyGlow(HANDLE hProcess, uint16_t entityIndex, Entity* entityList, const Entity& local);
        static void FunctionGlow(HANDLE hProcess, Entity* entityList, const Entity& local);
        static void BlueXrayGlow(HANDLE hProcess, HMODULE clientModule, BOOL isOn);

    private:
        static DWORD32 FindBlueXrayAddress(HANDLE hProcess, HMODULE clientModule, uint64_t& blueXrayAddr);
    };

} // namespace CS2Assist

#endif // CS2ASSIST_GLOW_ESP_H