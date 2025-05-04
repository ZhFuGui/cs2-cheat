#include "../../headers/system/GameSystemMgr.h"
#include "../../output/offsets.hpp"
#include "../../headers/core/ProcessUtil.h"
#include "../../headers/utils/ConstsUtil.h"
#include <iostream>
namespace CS2Assist {

    GameSystemMgr::GameSystemMgr(HANDLE processHandle, HMODULE clientModule)
        : hProcess(processHandle), ClientModuleAddress((uint64_t)clientModule){
    }
    
    GameSystemMgr::~GameSystemMgr() {
    }

    bool GameSystemMgr::GetSensitivity(GameSystem& gameSystem) const {

        uintptr_t pSensitivity{ 0 };
        
        if (
            !ReadProcessMemory(
                hProcess,
                LPCVOID(ClientModuleAddress + cs2_dumper::offsets::client_dll::dwSensitivity),
                &pSensitivity,
                sizeof(pSensitivity),
                nullptr)
            ||
            !pSensitivity
            )
        {
            return false;
        };

        float sensitivityBuffer{ 1.0f };

        if (
            !ReadProcessMemory(
                hProcess,
                LPCVOID(pSensitivity + cs2_dumper::offsets::client_dll::dwSensitivity_sensitivity),
                &sensitivityBuffer,
                sizeof(sensitivityBuffer),
                nullptr)
            ||
            sensitivityBuffer<0.09f
            ||
            sensitivityBuffer>8.1f
            )
        {
            return false;
        };

        gameSystem.sensitivity = sensitivityBuffer;

        return true;
    }

    // 实现获取地图名的逻辑
    bool GameSystemMgr::GetMapName(GameSystem& gameSystem) const {

        uintptr_t pstrMapStateInfo{ 0 };

        char buffer[MAX_PATH]{ 0 };

        if (
            !ProcessUtil::ScanSignature(hProcess, (HMODULE)ClientModuleAddress, Consts::SignCode::strMapStateInfo, pstrMapStateInfo)
            ||
            !pstrMapStateInfo
            ||
            !ReadProcessMemory(
                hProcess,
                LPCVOID(pstrMapStateInfo + Consts::SignCode::strMapStateInfo_size),
                &buffer,
                sizeof(buffer) - 1,       //保证buffer最后一位是0
                nullptr)
            ||
            !buffer
            )
        {
            return false;
        };

        gameSystem.mapStateInfo = (std::string)buffer;

        return true;
    };

    bool GameSystemMgr::GetGameState(GameSystem& gameSystem) const {

        gameSystem.gameState = GameState::InGame;

        return false;
    }

    std::pair<int, int> GameSystemMgr::GetScore() const {
        // 实现获取对局比分的逻辑
        return { 0, 0 }; // 示例返回值
    }

    void GameSystemMgr::Update(GameSystem& gameSystem) {
        
        GetSensitivity(gameSystem);
        GetMapName(gameSystem);
    }

} // namespace CS2Assist