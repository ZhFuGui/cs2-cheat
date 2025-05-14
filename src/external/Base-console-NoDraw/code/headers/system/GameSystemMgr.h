#pragma once
#ifndef CS2ASSIST_GAME_SYSTEM_MGR_H
#define CS2ASSIST_GAME_SYSTEM_MGR_H

#include <string>
#include <vector>
#include <Windows.h>
namespace CS2Assist {

    // ��Ϸ״̬ö��
    enum class GameState {
        InGame,     // ��Ϸ��
        Loading,    // ������
        Lobby,      // ������
        Unknown     // δ֪״̬
    };

    // ����ʵ�壨�������ԣ�
    struct SmokeEntity {
        float x, y, z;  // λ������
        float duration; // ����ʱ�䣨�룩
    };

    // ���ⵯʵ�壨�������ԣ�
    struct FlashEntity {
        float x, y, z;  // λ������
        float explodeTime; // ��ըʱ�䣨�룩
    };

    // ǹ֧ʵ�壨�������ԣ�
    struct WeaponEntity {
        std::string type; // ǹ֧���ͣ��� "AK47"��
        int ammo;         // ��ҩ����
    };

    struct GameSystem {
        GameState gameState;
        float sensitivity;
        std::string mapStateInfo;
        GameSystem() :
            gameState(GameState::Unknown),
            sensitivity(0.0f),
            mapStateInfo("unknown") {
        };

    };

    class GameSystemMgr {
    public:
        GameSystemMgr(HANDLE processHandle, HMODULE clientModule);
        ~GameSystemMgr();

        // ������Ϸϵͳ����
        void Update(GameSystem& gameSystem);

    private:
        HANDLE hProcess;
        uint64_t ClientModuleAddress;

        GameSystem gameSystem;

        // ��ȡ��Ϸ������
        bool GetSensitivity(GameSystem& gameSystem) const;
        
        // ��ȡ��Ļ���
        int GetScreenWidth() const;
        
        // ��ȡ��Ļ�߶�
        int GetScreenHeight() const;
        
        // ��ȡ��Ϸ״̬
        bool GetGameState(GameSystem& gameSystem) const;
        // ��ȡ�Ծֱȷ֣�����1�÷�, ����2�÷֣�
        std::pair<int, int> GetScore() const;
        // ��ȡ��Ϸ��ͼ��
        bool GetMapName(GameSystem& gameSystem) const;
        // ��ȡ�����б�
        std::vector<SmokeEntity> GetSmokeEntities() const;
        // ��ȡ���ⵯ�б�
        std::vector<FlashEntity> GetFlashEntities() const;
        // ��ȡǹ֧�б�
        std::vector<WeaponEntity> GetWeaponEntities() const;
    };

} // namespace CS2Assist

#endif // CS2ASSIST_GAME_SYSTEM_MGR_H