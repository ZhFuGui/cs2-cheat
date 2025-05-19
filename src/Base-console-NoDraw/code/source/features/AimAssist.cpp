#include "../../headers/features/AimAssist.h"
#include "../../headers/core/ProcessUtil.h"
#include "../../headers/features/GlowEsp.h"
#include "../../output/offsets.hpp"
#include "../../output/client_dll.hpp"

#include <cmath>
#include <memory>
#include <iostream>

namespace CS2Assist {

    AimAssist::AimAssist(HANDLE processHandle, HMODULE clientModule, GameSystem& gameSystem,EntityMgr& entityMgr, Entity* entityList, Entity& local)
        : hProcess(processHandle), 
        ClientModuleAddress((uint64_t)clientModule),
        gameSystem(gameSystem),
        entityList(entityList), 
        local(local), 
        entityMgr(entityMgr) {
    }

    //��һ���������������ʱ�������������ᱻ�Զ�������������Դ,�˴�������AimAssist���Ѿ���.h�ļ��ж���
    AimAssist::~AimAssist() {
        StopAim(); // ȷ��������ʱֹͣ���鲢������Դ
    }

    void AimAssist::StartAim(TargetMode mode, const TargetScope& scope, AimControl::AimType type) {

        StopAim(); // ��ֹͣ��������

        control.mode = mode;
        control.scope = scope;
        control.type = type;
        control.isActive = true;

        switch (type) {
        case AimControl::AimType::Mouse:
            aimThread = std::thread(&AimAssist::MouseAim, this);
            break;
        case AimControl::AimType::Memory:
            aimThread = std::thread(&AimAssist::MemoryAim, this);
            break;
        case AimControl::AimType::Silent:
            aimThread = std::thread(&AimAssist::SilentAim, this);
            break;
        default:
            control.isActive = false;
            return;
        }

        aimThread.detach();

        targetGlowThread = std::thread(&AimAssist::TargetGlow, this);
        targetGlowThread.detach();
    }
    
    void AimAssist::StopAim() {

        control.isActive = false;

        if (targetGlowThread.joinable()) {
            targetGlowThread.join();
        }

        if (aimThread.joinable()) {
            aimThread.join();
        }

        // ���� SilentAim ����Դ������ԭʼ����
        if (silentShellcodeAddr) {
            if (silentHookAddr) {
                SIZE_T bytesWritten;
                WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(silentHookAddr),
                    originalCode, Consts::SilentAim::JmpPatchSize, &bytesWritten);
            }
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            silentHookAddr = 0;
        }
    }
    
    //��׼Ŀ�귢��
    void AimAssist::TargetGlow() {
        while (control.isActive) {
            for (int i = 0; i < 64; ++i) {
                if (entityList[i].isTarget) {
                    GlowEsp::ApplyGlow(hProcess, entityList[i].index, entityList, local);
                }
            }
        }
    }
    
    //ģ������ƶ����飬�ƶ����ش�С��Ҫ����ʵ��ƥ�䡣һ����˵�÷����׳���Ļ����bug�������Ǽ���ԭ��:1.�ƶ��������Ӽ��㲻׼ȷ��������豸��ϵͳ�����йء�2.���ڴ��ٶ�̫������������ƶ����ȡ����Ϸ�������������¡�
    void AimAssist::MouseAim() {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;

        while (control.isActive) {
            Entity target = SelectTarget(control.mode, control.scope);
            if (!target.isValid || target.health <= 0) {
                Sleep(5);
                continue;
            }

            Angle correction = CalcAimAngle(
                Target2Me(local.cameraPosition, PosCalibrated(target.cameraPosition, Angle2Arc(target.eyeAngle))),
                local.eyeAngle
            );

            input.mi.dx = static_cast<LONG>(correction.yaw * Consts::Factor::PerAngleInFov_scalingFactor / gameSystem.sensitivity);
            input.mi.dy = static_cast<LONG>(correction.pitch * Consts::Factor::PerAngleInFov_scalingFactor / gameSystem.sensitivity);
            SendInput(1, &input, sizeof(INPUT));
            Sleep(5);
        }
    }

    //�ڴ�����
    void AimAssist::MemoryAim() {
        while (control.isActive) {
            Entity target = SelectTarget(control.mode, control.scope);
            if (!target.isValid || target.health <= 0) {
                Sleep(10);
                continue;
            }

            Angle correction = Target2Me(local.cameraPosition, PosCalibrated(target.cameraPosition, Angle2Arc(target.eyeAngle)));


            WriteProcessMemory(hProcess,
                reinterpret_cast<LPVOID>(ClientModuleAddress + cs2_dumper::offsets::client_dll::dwViewAngles),
                &correction.pitch, sizeof(float), nullptr);
            WriteProcessMemory(hProcess,
                reinterpret_cast<LPVOID>(ClientModuleAddress + cs2_dumper::offsets::client_dll::dwViewAngles + 0x4),
                &correction.yaw, sizeof(float), nullptr);
            //Sleep(0);
        }
    }

    //��Ĭ���飬�״���VAC(ȫ����ȴ20��Сʱ)
    void AimAssist::SilentAim() {

        //Ϊshellcode�ڳ��ռ�
        silentShellcodeAddr = VirtualAllocEx(hProcess, NULL, 0xA00, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);  

        //�ж��Ƿ������ڴ�ɹ�
        if (!silentShellcodeAddr) {
            control.isActive = false;
            return;
        }

        //shellcode�ĸ��ƣ����︴��һ��shellcode����Ϊ����Ҫ��shellcode�����޸Ĵ������ʹ�á�
        BYTE shellcode[Consts::SilentAim::ShellcodeSize];                                                                   
        memcpy(shellcode, Consts::SilentAim::Shellcode, Consts::SilentAim::ShellcodeSize);

        //��Ҫ��ת��ƫ�ƴ�С����λ��Ĭ����0x500,��ô����PitchOffsetPos+pitchOffset+sizeof(Pitch)=0x500�õ�ʽ���Եõ�pitchOffset,yawOffset
        uint32_t pitchOffset = Consts::SilentAim::AngleOffset - 0x4 - Consts::SilentAim::PitchOffsetPos;
        uint32_t yawOffset = Consts::SilentAim::AngleOffset - Consts::SilentAim::YawOffsetPos;
        memcpy(shellcode + Consts::SilentAim::PitchOffsetPos, &pitchOffset, sizeof(pitchOffset));
        memcpy(shellcode + Consts::SilentAim::YawOffsetPos, &yawOffset, sizeof(yawOffset));

        //Ѱ�ұ�hook�ĵ�ַ
        ProcessUtil::ScanSignature(hProcess, reinterpret_cast<HMODULE>(ClientModuleAddress),Consts::SignCode::ServerShotAngle, silentHookAddr);

        //�ж��Ƿ��ҵ���ַ
        if (!silentHookAddr) {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        };
        
        // ȷ�����ص�ַ
        uintptr_t returnAddr = silentHookAddr + Consts::SilentAim::JmpPatchSize;
        memcpy(shellcode + Consts::SilentAim::ReturnAddrPos, &returnAddr, sizeof(uintptr_t));

        // ����ԭʼ����,���ڻظ���Ĭ����
        SIZE_T bytesRead;
        if (!ReadProcessMemory(hProcess,
            reinterpret_cast<LPVOID>(silentHookAddr),
            originalCode,
            Consts::SilentAim::JmpPatchSize,
            &bytesRead)
            ||
            bytesRead != Consts::SilentAim::JmpPatchSize
            )
        {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        };

        //д��shellcode
        SIZE_T bytesWritten;
        if (!WriteProcessMemory(hProcess, silentShellcodeAddr, shellcode, Consts::SilentAim::ShellcodeSize, &bytesWritten)) {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        }

        //������ת���ܵ�shellcode
        BYTE jmpPatch[Consts::SilentAim::JmpPatchSize];
        memcpy(jmpPatch, Consts::SilentAim::JmpPatch, Consts::SilentAim::JmpPatchSize);
        memcpy(jmpPatch + Consts::SilentAim::ShellcodeAddrPos, &silentShellcodeAddr, sizeof(uintptr_t));

        //����originalCode
        if (!WriteProcessMemory(hProcess,
            reinterpret_cast<LPVOID>(silentHookAddr),
            jmpPatch,
            Consts::SilentAim::JmpPatchSize,
            &bytesWritten))
        {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        };

        //������д����ڴ��ַ���ڴ��и�����λ��ƫ����ǰ�棬����������pitch
        uintptr_t pitchAddr = reinterpret_cast<uintptr_t>(silentShellcodeAddr) + Consts::SilentAim::AngleOffset;

        //д��ŷ���ǵ�����ѭ������ʵ������벻��while���ֲ�������Σ�յķ������Գ���hook��Ϸ�и���ŷ���ǵ�.text����д��shellcode���뱾��Ĭʵ�ֵ�hookԭ��һ�£�����Ͳ�������
        while (control.isActive) {

            //����ʵ����������ȷ������ģʽ
            Entity target = SelectTarget(control.mode, control.scope);
            
            if (!target.isValid || target.health <= 0) {
                Sleep(10);
                continue;
            };

            //��������Ƕ�
            Angle correction = Target2Me(local.cameraPosition, PosCalibrated(target.cameraPosition, Angle2Arc(target.eyeAngle)));

            //��angleд�����ȡ�ڴ�
            WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(pitchAddr), &correction, sizeof(correction), nullptr);

            //�˴������ӳٷ������
            Sleep(0);
        }
    }

    //ѡ����׼Ŀ��
    Entity AimAssist::SelectTarget(TargetMode mode, const TargetScope& scope) const {

        Entity bestTarget;
        float bestMetric = FLT_MAX;

        // ����Ѱ�����Ŀ��
        for (int i = 0; i < 64; ++i) {
            const Entity& entity = entityList[i];
            if (!entity.isValid || entity.health <= 0 || entity.controllerAddr == local.controllerAddr) continue;

            bool validType = false;
            switch (scope.type) {
            case TargetScope::TargetType::EnemiesOnly: validType = entity.teamId != local.teamId; break;
            case TargetScope::TargetType::TeammatesOnly: validType = entity.teamId == local.teamId; break;
            case TargetScope::TargetType::Indiscriminate: validType = true; break;
            }
            if (!validType) continue;

            if (!IsInFov(entity, scope.horizontalFov, scope.verticalFov)) continue;

            float metric = 0.0f;
            switch (mode) {
            case TargetMode::ClosestDistance:
                metric = local.cameraPosition.DistanceXY(entity.cameraPosition);
                break;
            case TargetMode::WithinCrosshair: {
                Angle targetAngle = Target2Me(local.cameraPosition, entity.cameraPosition);
                Angle diff = CalcAimAngle(targetAngle, local.eyeAngle);
                metric = std::sqrt(diff.pitch * diff.pitch + diff.yaw * diff.yaw);
                break;
            }
            case TargetMode::VisibleEntity:
                if (!IsMeVisible(entity)&&!IsTargetVisible(entity)) continue;
                metric = local.cameraPosition.DistanceXY(entity.cameraPosition);
                break;
            case TargetMode::SpecificName: continue; // δʵ�֣�����
            case TargetMode::LowestHealth:
                metric = static_cast<float>(entity.health);
                bestMetric = -FLT_MAX; // �������Ѫ������ʼֵΪ������
                break;
            case TargetMode::KnifeOrTaser: continue; // δʵ�֣�����
            }

            if ((mode == TargetMode::LowestHealth && metric > bestMetric) ||
                (mode != TargetMode::LowestHealth && metric < bestMetric)) {
                bestMetric = metric;
                bestTarget = entity;
            }
        }

        // ����ҵ������Ŀ�꣬���� isTarget ��Ϊ 1������Ϊ0
        if (bestTarget.isValid) {
            for (int i = 0; i < 64; ++i) {
                entityList[i].isTarget = (entityList[i].controllerAddr == bestTarget.controllerAddr &&
                    entityList[i].pawnAddr == bestTarget.pawnAddr);
            }
            entityMgr.ReadEntityData(bestTarget.controllerAddr, bestTarget.pawnAddr, bestTarget);
        }
        else {
            for (int i = 0; i < 64; ++i) {
                entityList[i].isTarget = false;
            }
        }

        return bestTarget;
    };

    //�Ƿ���Ҫ���������
    bool AimAssist::IsInFov(const Entity& entity, float horizontalFov, float verticalFov) const {
        Angle targetAngle = Target2Me(local.cameraPosition, entity.cameraPosition);
        Angle diff = CalcAimAngle(targetAngle, local.eyeAngle);
        return std::abs(diff.yaw) <= horizontalFov && std::abs(diff.pitch) <= verticalFov;
    }
    
    //Ŀ���Ƿ��ҿɼ�(��ǰ��������)
    bool AimAssist::IsTargetVisible(const Entity& entity) const {
        return entity.isSpotted;
    }

    //Ŀ���Ƿ񿴼�����
    bool AimAssist::IsMeVisible(const Entity& entity) const {
        return (local.SpottedByMask & (DWORD64(1) << (entity.index - 1)));
    }

    //�Ƕ�ת����
    Angle AimAssist::Angle2Arc(const Angle& angle) const {
        return Angle(
            angle.pitch * (static_cast<float>(Consts::Math::PI) / 180.0f),
            angle.yaw * (static_cast<float>(Consts::Math::PI) / 180.0f)
        );
    }

    //��׼λ������
    Vec3 AimAssist::PosCalibrated(const Vec3& pos, const Angle& eyeArc) const {
        return Vec3(
            pos.x + std::cos(eyeArc.yaw) * Consts::Factor::camera2head_Factor,
            pos.y + std::sin(eyeArc.yaw) * Consts::Factor::camera2head_Factor,
            pos.z
        );
    }

    //������Ҫƫת�ĽǶȣ����ڴ棬���������Ҫ�õ�
    Angle AimAssist::CalcAimAngle(const Angle& targetAngle, const Angle& myEyeAngle) const {

        float dYaw = targetAngle.yaw - myEyeAngle.yaw;
        float dPitch = targetAngle.pitch - myEyeAngle.pitch;

        if (dYaw > 180.0f) dYaw -= 360.0f;
        else if (dYaw < -180.0f) dYaw += 360.0f;

        return Angle(dPitch, -dYaw);
    };

    //����Ŀ��������3D�����еĽǶ�(����������)
    Angle AimAssist::Target2Me(const Vec3& myPos, const Vec3& targetPos) const {

        float dX = targetPos.x - myPos.x;
        float dY = targetPos.y - myPos.y;
        float dZ = targetPos.z - myPos.z;
        float dXY = myPos.DistanceXY(targetPos);

        float yaw = std::atan2(dY, dX) * 180.0f / static_cast<float>(Consts::Math::PI);
        float pitch = std::atan2(-dZ, dXY) * 180.0f / static_cast<float>(Consts::Math::PI);

        if (control.isRecoilControl) {
            Angle RecoilCompensation;

            ReadProcessMemory(hProcess, reinterpret_cast<LPVOID>(local.pawnAddr+ cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle), &RecoilCompensation, sizeof(RecoilCompensation), nullptr);

            pitch -= RecoilCompensation.pitch * 2.0f;
            yaw -= RecoilCompensation.yaw * 2.0f;
        }

        return Angle(pitch, yaw);
    }

} // namespace CS2Assist