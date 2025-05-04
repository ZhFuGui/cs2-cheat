#ifndef CS2ASSIST_AIM_ASSIST_H
#define CS2ASSIST_AIM_ASSIST_H

#include <Windows.h>
#include <thread>
#include <atomic>
#include "../entity/EntityMgr.h"
#include "../utils/ConstsUtil.h"
#include "../system/GameSystemMgr.h"
namespace CS2Assist {

    enum class TargetMode {
        ClosestDistance,    // 最近距离
        WithinCrosshair,    // 准心范围内
        VisibleEntity,      // 可视实体（无掩体）
        SpecificName,       // 特定名称
        LowestHealth,       // 血量最低
        KnifeOrTaser        // 持刀或电击枪优先
    };

    struct TargetScope {
        enum class TargetType {
            EnemiesOnly,    // 仅敌人
            TeammatesOnly,  // 仅队友
            Indiscriminate  // 无差别（除自己外）
        } type = TargetType::EnemiesOnly;

        float horizontalFov = 180.0f; // 水平视角范围 [0, 180] 度
        float verticalFov = 90.0f;    // 垂直视角范围 [0, 90] 度
    };

    struct AimControl {
        std::atomic<bool> isRecoilControl{ true }; // 压枪是否开启
        std::atomic<bool> isActive{ false }; // 自瞄是否开启
        TargetMode mode{ TargetMode::ClosestDistance };
        TargetScope scope{};
        enum class AimType { None, Mouse, Memory, Silent } type{ AimType::None };
    };

    class AimAssist {
    public:
        AimAssist(HANDLE processHandle, HMODULE clientModule, GameSystem& gameSystem,EntityMgr& entityMgr, Entity* entityList, Entity& local);
        ~AimAssist();

        void StartAim(
            TargetMode mode = TargetMode::ClosestDistance,
            const TargetScope& scope = TargetScope(),
            AimControl::AimType type = AimControl::AimType::Mouse);

        void StopAim();

    private:
        HANDLE hProcess;
        uint64_t ClientModuleAddress;
        GameSystem& gameSystem;
        EntityMgr& entityMgr;
        Entity* entityList;
        Entity& local;
        AimControl control;                                     // 自瞄控制状态
        std::thread aimThread;                                  // 自瞄线程
        std::thread targetGlowThread;                           // 目标发光线程
        LPVOID silentShellcodeAddr{ nullptr };                  // SilentAim 的 shellcode 地址
        uint64_t silentHookAddr{ 0 };                           // SilentAim 的注入目标地址
        BYTE originalCode[Consts::SilentAim::JmpPatchSize]{};   // 保存被覆盖的原始代码

        void MouseAim();
        void MemoryAim();
        void SilentAim();

        void TargetGlow();

        Entity SelectTarget(TargetMode mode, const TargetScope& scope) const;

        bool IsInFov(const Entity& entity, float horizontalFov, float verticalFov) const;
        bool IsTargetVisible(const Entity& entity) const;
        bool IsMeVisible(const Entity& entity) const;
        Angle Angle2Arc(const Angle& angle) const;
        Vec3 PosCalibrated(const Vec3& pos, const Angle& eyeArc) const;
        Angle CalcAimAngle(const Angle& targetAngle, const Angle& myEyeAngle) const;
        Angle Target2Me(const Vec3& myPos, const Vec3& targetPos) const;
    };

} // namespace CS2Assist

#endif // CS2ASSIST_AIM_ASSIST_H