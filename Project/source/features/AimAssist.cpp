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

    //当一个对象的生命结束时，其析构函数会被自动调用以清理资源,此处对象是AimAssist，已经在.h文件中定义
    AimAssist::~AimAssist() {
        StopAim(); // 确保在析构时停止自瞄并清理资源
    }

    void AimAssist::StartAim(TargetMode mode, const TargetScope& scope, AimControl::AimType type) {

        StopAim(); // 先停止现有自瞄

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

        // 清理 SilentAim 的资源并回填原始代码
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
    
    //瞄准目标发光
    void AimAssist::TargetGlow() {
        while (control.isActive) {
            for (int i = 0; i < 64; ++i) {
                if (entityList[i].isTarget) {
                    GlowEsp::ApplyGlow(hProcess, entityList[i].index, entityList, local);
                }
            }
        }
    }
    
    //模拟鼠标移动自瞄，移动像素大小需要根据实际匹配。一般来说该方法易出屏幕抖动bug，下面是几个原因:1.移动缩放因子计算不准确，与具体设备与系统设置有关。2.读内存速度太慢，导致鼠标移动后读取的游戏数据来不及更新。
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

    //内存自瞄
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

    //静默自瞄，易触发VAC(全球冷却20个小时)
    void AimAssist::SilentAim() {

        //为shellcode腾出空间
        silentShellcodeAddr = VirtualAllocEx(hProcess, NULL, 0xA00, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);  

        //判断是否申请内存成功
        if (!silentShellcodeAddr) {
            control.isActive = false;
            return;
        }

        //shellcode的复制，这里复制一份shellcode是因为还需要对shellcode进行修改处理才能使用。
        BYTE shellcode[Consts::SilentAim::ShellcodeSize];                                                                   
        memcpy(shellcode, Consts::SilentAim::Shellcode, Consts::SilentAim::ShellcodeSize);

        //需要跳转的偏移大小，该位置默认是0x500,那么根据PitchOffsetPos+pitchOffset+sizeof(Pitch)=0x500该等式可以得到pitchOffset,yawOffset
        uint32_t pitchOffset = Consts::SilentAim::AngleOffset - 0x4 - Consts::SilentAim::PitchOffsetPos;
        uint32_t yawOffset = Consts::SilentAim::AngleOffset - Consts::SilentAim::YawOffsetPos;
        memcpy(shellcode + Consts::SilentAim::PitchOffsetPos, &pitchOffset, sizeof(pitchOffset));
        memcpy(shellcode + Consts::SilentAim::YawOffsetPos, &yawOffset, sizeof(yawOffset));

        //寻找被hook的地址
        ProcessUtil::ScanSignature(hProcess, reinterpret_cast<HMODULE>(ClientModuleAddress),Consts::SignCode::ServerShotAngle, silentHookAddr);

        //判断是否找到地址
        if (!silentHookAddr) {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        };
        
        // 确定返回地址
        uintptr_t returnAddr = silentHookAddr + Consts::SilentAim::JmpPatchSize;
        memcpy(shellcode + Consts::SilentAim::ReturnAddrPos, &returnAddr, sizeof(uintptr_t));

        // 保存原始代码,用于回复静默自瞄
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

        //写入shellcode
        SIZE_T bytesWritten;
        if (!WriteProcessMemory(hProcess, silentShellcodeAddr, shellcode, Consts::SilentAim::ShellcodeSize, &bytesWritten)) {
            VirtualFreeEx(hProcess, silentShellcodeAddr, 0, MEM_RELEASE);
            silentShellcodeAddr = nullptr;
            control.isActive = false;
            return;
        }

        //复制跳转功能的shellcode
        BYTE jmpPatch[Consts::SilentAim::JmpPatchSize];
        memcpy(jmpPatch, Consts::SilentAim::JmpPatch, Consts::SilentAim::JmpPatchSize);
        memcpy(jmpPatch + Consts::SilentAim::ShellcodeAddrPos, &silentShellcodeAddr, sizeof(uintptr_t));

        //覆盖originalCode
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

        //俯仰角写入的内存地址，内存中俯仰角位于偏航角前面，所以命名用pitch
        uintptr_t pitchAddr = reinterpret_cast<uintptr_t>(silentShellcodeAddr) + Consts::SilentAim::AngleOffset;

        //写入欧拉角的数据循环。事实上如果想不用while这种不优雅且危险的方法可以尝试hook游戏中更新欧拉角的.text区段写入shellcode，与本静默实现的hook原理一致，这里就不做处理。
        while (control.isActive) {

            //根据实例化的数据确定索敌模式
            Entity target = SelectTarget(control.mode, control.scope);
            
            if (!target.isValid || target.health <= 0) {
                Sleep(10);
                continue;
            };

            //计算射击角度
            Angle correction = Target2Me(local.cameraPosition, PosCalibrated(target.cameraPosition, Angle2Arc(target.eyeAngle)));

            //将angle写入待读取内存
            WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(pitchAddr), &correction, sizeof(correction), nullptr);

            //此处设置延迟方便调试
            Sleep(0);
        }
    }

    //选择瞄准目标
    Entity AimAssist::SelectTarget(TargetMode mode, const TargetScope& scope) const {

        Entity bestTarget;
        float bestMetric = FLT_MAX;

        // 遍历寻找最佳目标
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
            case TargetMode::SpecificName: continue; // 未实现，跳过
            case TargetMode::LowestHealth:
                metric = static_cast<float>(entity.health);
                bestMetric = -FLT_MAX; // 对于最低血量，初始值为负无穷
                break;
            case TargetMode::KnifeOrTaser: continue; // 未实现，跳过
            }

            if ((mode == TargetMode::LowestHealth && metric > bestMetric) ||
                (mode != TargetMode::LowestHealth && metric < bestMetric)) {
                bestMetric = metric;
                bestTarget = entity;
            }
        }

        // 如果找到了最佳目标，将其 isTarget 置为 1，否则为0
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

    //是否在要求的视线内
    bool AimAssist::IsInFov(const Entity& entity, float horizontalFov, float verticalFov) const {
        Angle targetAngle = Target2Me(local.cameraPosition, entity.cameraPosition);
        Angle diff = CalcAimAngle(targetAngle, local.eyeAngle);
        return std::abs(diff.yaw) <= horizontalFov && std::abs(diff.pitch) <= verticalFov;
    }
    
    //目标是否被我可见(当前代码有误)
    bool AimAssist::IsTargetVisible(const Entity& entity) const {
        return entity.isSpotted;
    }

    //目标是否看见了我
    bool AimAssist::IsMeVisible(const Entity& entity) const {
        return (local.SpottedByMask & (DWORD64(1) << (entity.index - 1)));
    }

    //角度转弧度
    Angle AimAssist::Angle2Arc(const Angle& angle) const {
        return Angle(
            angle.pitch * (static_cast<float>(Consts::Math::PI) / 180.0f),
            angle.yaw * (static_cast<float>(Consts::Math::PI) / 180.0f)
        );
    }

    //瞄准位置修正
    Vec3 AimAssist::PosCalibrated(const Vec3& pos, const Angle& eyeArc) const {
        return Vec3(
            pos.x + std::cos(eyeArc.yaw) * Consts::Factor::camera2head_Factor,
            pos.y + std::sin(eyeArc.yaw) * Consts::Factor::camera2head_Factor,
            pos.z
        );
    }

    //计算需要偏转的角度，仅内存，鼠标自瞄需要用到
    Angle AimAssist::CalcAimAngle(const Angle& targetAngle, const Angle& myEyeAngle) const {

        float dYaw = targetAngle.yaw - myEyeAngle.yaw;
        float dPitch = targetAngle.pitch - myEyeAngle.pitch;

        if (dYaw > 180.0f) dYaw -= 360.0f;
        else if (dYaw < -180.0f) dYaw += 360.0f;

        return Angle(dPitch, -dYaw);
    };

    //计算目标与我在3D世界中的角度(带后座修正)
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