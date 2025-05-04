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
        ClosestDistance,    // �������
        WithinCrosshair,    // ׼�ķ�Χ��
        VisibleEntity,      // ����ʵ�壨�����壩
        SpecificName,       // �ض�����
        LowestHealth,       // Ѫ�����
        KnifeOrTaser        // �ֵ�����ǹ����
    };

    struct TargetScope {
        enum class TargetType {
            EnemiesOnly,    // ������
            TeammatesOnly,  // ������
            Indiscriminate  // �޲�𣨳��Լ��⣩
        } type = TargetType::EnemiesOnly;

        float horizontalFov = 180.0f; // ˮƽ�ӽǷ�Χ [0, 180] ��
        float verticalFov = 90.0f;    // ��ֱ�ӽǷ�Χ [0, 90] ��
    };

    struct AimControl {
        std::atomic<bool> isRecoilControl{ true }; // ѹǹ�Ƿ���
        std::atomic<bool> isActive{ false }; // �����Ƿ���
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
        AimControl control;                                     // �������״̬
        std::thread aimThread;                                  // �����߳�
        std::thread targetGlowThread;                           // Ŀ�귢���߳�
        LPVOID silentShellcodeAddr{ nullptr };                  // SilentAim �� shellcode ��ַ
        uint64_t silentHookAddr{ 0 };                           // SilentAim ��ע��Ŀ���ַ
        BYTE originalCode[Consts::SilentAim::JmpPatchSize]{};   // ���汻���ǵ�ԭʼ����

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