#ifndef CS2ASSIST_GLOW_ESP_H
#define CS2ASSIST_GLOW_ESP_H

#include <Windows.h>
#include <atomic>
#include "../entity/EntityMgr.h"
#include "../core/ProcessUtil.h"
#include "../utils/ConstsUtil.h"

namespace CS2Assist {

    enum class GlowMode {
        BlueXray,      // 蓝色光透视
        GlowFunction   // 基于游戏发光函数的透视
    };

    // ARGB 颜色结构体
    struct Color {
        uint8_t a; // Alpha (透明度/亮度)
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

    // 全局静态透视配置
    namespace GlowConfig {
        inline std::atomic<bool> isActive{ true };              // 透视是否开启
        inline GlowMode mode{ GlowMode::GlowFunction };         // 当前透视模式
        inline bool glowTeammates{ false };                     // 是否绘制队友
        inline bool glowEnemies{ true };                        // 是否绘制敌人
        inline Color teammateColor{ 255, 0, 255, 0 };           // 队友颜色（绿色）
        inline Color enemyColor{ 255, 0, 255, 255 };            // 敌人颜色（黄色）
        inline Color targetColor{ 255, 0, 0, 255 };             // 目标颜色（红色）
    };

    class GlowEsp {
    public:
        // 静态函数，直接使用全局配置
        static void ApplyGlow(HANDLE hProcess, uint16_t entityIndex, Entity* entityList, const Entity& local);
        static void FunctionGlow(HANDLE hProcess, Entity* entityList, const Entity& local);
        static void BlueXrayGlow(HANDLE hProcess, HMODULE clientModule, BOOL isOn);

    private:
        static DWORD32 FindBlueXrayAddress(HANDLE hProcess, HMODULE clientModule, uint64_t& blueXrayAddr);
    };

} // namespace CS2Assist

#endif // CS2ASSIST_GLOW_ESP_H