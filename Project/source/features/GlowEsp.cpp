#include "../../headers/features/GlowEsp.h"
#include "../../output/offsets.hpp"
#include "../../output/client_dll.hpp"
#include <iostream>

namespace CS2Assist {

    //利用游戏发光函数应用到指定实体的发光
    void GlowEsp::ApplyGlow(HANDLE hProcess, uint16_t entityIndex, Entity* entityList, const Entity& local) {
        // 根据 Index 找到对应的实体
        const Entity* targetEntity = &entityList[entityIndex];

        // 如果实体无效、健康值为 0 或是本地玩家，直接返回
        if (!targetEntity || !targetEntity->isValid || targetEntity->health <= 0 ||
            targetEntity->controllerAddr == local.controllerAddr) {
            return;
        }

        // 计算发光相关的内存地址
        uint64_t glowColorOverride = targetEntity->pawnAddr +
            cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow +
            cs2_dumper::schemas::client_dll::CGlowProperty::m_glowColorOverride;
        uint64_t glowFunc = targetEntity->pawnAddr +
            cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow +
            cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing;

        // 设置发光状态
        BOOL bGlowing = true;
        uint32_t color;

        // 如果 isTarget 为 true，无视所有条件，使用 targetColor 发光
        if (targetEntity->isTarget) {
            color = GlowConfig::targetColor.ToUInt32();
        }
        else {
            // 判断是否为队友并决定是否发光
            bool isTeammate = targetEntity->teamId == local.teamId;
            bool shouldGlow = (isTeammate && GlowConfig::glowTeammates) ||
                (!isTeammate && GlowConfig::glowEnemies);
            if (!shouldGlow) return;

            // 根据队伍设置颜色
            color = isTeammate ? GlowConfig::teammateColor.ToUInt32() : GlowConfig::enemyColor.ToUInt32();
        }

        // 写入内存
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(glowColorOverride),
            &color, sizeof(color), nullptr);
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(glowFunc),
            &bGlowing, sizeof(bGlowing), nullptr);
    }

    //全员发光
    void GlowEsp::FunctionGlow(HANDLE hProcess, Entity* entityList, const Entity& local) {

        //使所有实体发光
        for (int i = 0; i < 64; ++i) {
            if (entityList[i].isValid) {
                ApplyGlow(hProcess, i, entityList, local);
            }
        }
        return;
    };

    //全员发光
    void GlowEsp::BlueXrayGlow(HANDLE hProcess, HMODULE clientModule, BOOL isOn) {

        //先找到地址，然后判断传入isOn参数，如果true那么写入nop，否则写入origin

        uint64_t blueXrayAddr = 0;
        DWORD32 patchResult = FindBlueXrayAddress(hProcess, clientModule, blueXrayAddr);

        if (!patchResult) {
            std::cerr << "Failed to find BlueXray address" << std::endl;
            GlowConfig::isActive = false;
            return;
        }

        // 确定要写入的数据
        const BYTE* patchData = isOn ? Consts::SignCode::noppedPatch : Consts::SignCode::originalPatch;
        size_t patchSize = sizeof(Consts::SignCode::noppedPatch); // 假设两种 Patch 长度一致

        SIZE_T bytesWritten;
        if (!WriteProcessMemory(
            hProcess,
            reinterpret_cast<LPVOID>(blueXrayAddr),
            patchData,
            patchSize,
            &bytesWritten) ||
            bytesWritten != patchSize)
        {
            std::cerr << "Failed to write patch at " << std::hex << blueXrayAddr << std::endl;
            GlowConfig::isActive = false;
            return;
        }

        // 输出状态
        if (isOn) {
            std::cout << "BlueXray enabled at " << std::hex << blueXrayAddr << std::endl;
        }
        else {
            std::cout << "BlueXray disabled at " << std::hex << blueXrayAddr << std::endl;
        }
    };

    //寻找xray地址
    DWORD32 GlowEsp::FindBlueXrayAddress(HANDLE hProcess, HMODULE clientModule, uint64_t& blueXrayAddr) {

        // 如果地址已找到，直接返回 TRUE
        if (blueXrayAddr) {
            return TRUE;
        }

        // 尝试第一个签名
        if (ProcessUtil::ScanSignature(hProcess, clientModule, Consts::SignCode::BlueXray, blueXrayAddr)) {
            return (DWORD32)2;
        }

        // 尝试第二个签名
        if (ProcessUtil::ScanSignature(hProcess, clientModule, Consts::SignCode::BlueXrayNopped, blueXrayAddr)) {
            return (DWORD32)3;
        }

        // 都失败，返回 FALSE
        std::cerr << "Failed to locate BlueXray address with both signatures" << std::endl;
        return FALSE;
    };

} // namespace CS2Assist