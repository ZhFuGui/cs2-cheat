#include "../../headers/features/AntiUtils.h"
#include "../../output/offsets.hpp"
#include "../../output/client_dll.hpp"
#include <iostream>

namespace CS2Assist {

    AntiUtils::AntiUtils(HANDLE processHandle, HMODULE clientModule, Entity& pLocalEntity)
        : hProcess(processHandle), ClientModuleAddress((uint64_t)clientModule) , local(pLocalEntity){
    }

    AntiUtils::~AntiUtils() {
    }

    // 读取实体类指针链
    uint64_t AntiUtils::GetBaseEntity(int index) const {

        uint64_t entitybase;
        ReadProcessMemory(hProcess, (LPVOID)(ClientModuleAddress + cs2_dumper::offsets::client_dll::dwEntityList), &entitybase, sizeof(entitybase), nullptr);

        uint64_t entitylsitbase;
        ReadProcessMemory(hProcess, (LPVOID)(entitybase + 0x8 * (index >> 9) + 16), &entitylsitbase, sizeof(entitylsitbase), nullptr);

        uint64_t pawnbase;
        ReadProcessMemory(hProcess, (LPVOID)(entitylsitbase + (0x78 * (index & 0x1ff))), &pawnbase, sizeof(entitylsitbase), nullptr);

        if (pawnbase)
        {
            return pawnbase;
        }
        else {
            return false;
        };
    };

    //遍历每一个system实体,通用处理方式
    void AntiUtils::ProcessEntities(void (*processFunc)(HANDLE, uint64_t)) {

        // 读取 entitySystem
        uint64_t entitySystem = 0;
        ReadProcessMemory(
            hProcess,
            (LPVOID)(ClientModuleAddress + cs2_dumper::offsets::client_dll::dwGameEntitySystem),
            &entitySystem,
            sizeof(entitySystem),
            nullptr);

        if (!entitySystem) return;

        // 读取 maxEntityIndex
        int maxEntityIndex = 0;
        ReadProcessMemory(
            hProcess,
            (LPVOID)(entitySystem + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex),
            &maxEntityIndex,
            sizeof(maxEntityIndex),
            nullptr);

        if (maxEntityIndex <= 0) return;

        //捕获每一个实体
        for (int i = 0; i <= maxEntityIndex; ++i) {
            uint64_t EntityInstanceAddr = GetBaseEntity(i);
            if (!EntityInstanceAddr) continue;

            processFunc(hProcess, EntityInstanceAddr);
        }
    }

    //移除烟雾效果
    void AntiUtils::RemoveSmoke() {
        //此处有炫技嫌疑，读者可自行利用LLM帮助理解
        ProcessEntities(
            [](HANDLE hProcess, uint64_t pEntityInstance)// -> void 可省略
            {
                /*
                namespace CEntityInstance {
                    constexpr std::ptrdiff_t m_iszPrivateVScripts               // CUtlSymbolLarge
                    constexpr std::ptrdiff_t m_pEntity                          //CEntityIdentity*
                    constexpr std::ptrdiff_t m_CScriptComponent                 // CScriptComponent*
                    constexpr std::ptrdiff_t m_bVisibleinPVS                    // bool
                }
                */
                uint64_t pEntityIdentity = 0;
                ReadProcessMemory(hProcess, (LPVOID)(pEntityInstance + 0x10), &pEntityIdentity, sizeof(pEntityIdentity), nullptr);
                if (!pEntityIdentity) return;
                /*
                namespace CEntityIdentity {
                    constexpr std::ptrdiff_t m_nameStringableIndex              // int32
                    constexpr std::ptrdiff_t m_name                             // CUtlSymbolLarge
                    constexpr std::ptrdiff_t m_designerName                     // CUtlSymbolLarge
                    constexpr std::ptrdiff_t m_flags                            // uint32
                    constexpr std::ptrdiff_t m_worldGroupId                     // WorldGroupId_t
                    constexpr std::ptrdiff_t m_fDataObjectTypes                 // uint32
                    constexpr std::ptrdiff_t m_PathIndex                        //ChangeAccessorFieldPathIndex_t
                    constexpr std::ptrdiff_t m_pPrev                            // CEntityIdentity*
                    constexpr std::ptrdiff_t m_pNex                             // CEntityIdentity*
                    constexpr std::ptrdiff_t m_pPrevByClass                     // CEntityIdentity*
                    constexpr std::ptrdiff_t m_pNextByClass                     // CEntityIdentity*
                }
                */
                uint64_t pDesignerName = 0;
                ReadProcessMemory(hProcess, (LPVOID)(pEntityIdentity + 0x20), &pDesignerName, sizeof(pDesignerName), nullptr);
                if (!pDesignerName) return;

                // 读取实体名称
                char nameBuffer[256] = { 0 };
                ReadProcessMemory(hProcess, (LPVOID)pDesignerName, nameBuffer, sizeof(nameBuffer) - 1, nullptr);
                std::string designerName = nameBuffer;

                if (designerName == "smokegrenade_projectile") {

                    float fZero = 0.0f;
                    WriteProcessMemory(hProcess, (LPVOID)(pEntityInstance + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin),
                        &fZero, sizeof(fZero), nullptr);

                    bool bfalse = false;
                    WriteProcessMemory(hProcess, (LPVOID)(pEntityInstance + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_bDidSmokeEffect),
                        &bfalse, sizeof(bfalse), nullptr);
                    WriteProcessMemory(hProcess, (LPVOID)(pEntityInstance + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_bSmokeEffectSpawned),
                        &bfalse, sizeof(bfalse), nullptr);

                    /*
                    实际上此处多次使用WriteProcessMemory显得冗杂，读者尝试更优雅的写法可以封装写内存操作为函数模板
                    template <typename T>
                    void WriteValue(HANDLE hProcess, uint64_t address, const T& value) {
                        WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), nullptr);
                    }
                    这样调用
                    WriteValue<float>(hProcess, pawnAddr + cs2_dumper::schemas::client_dll::C_SmokeGrenadeProjectile::m_nSmokeEffectTickBegin, 0.0f);
                    */
                };
            });
    };

    //移除闪光效果
    void AntiUtils::RemoveFlash() {
        float fZero = 0;
        WriteProcessMemory(hProcess, (LPVOID)(local.pawnAddr + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashDuration), &fZero, sizeof(fZero), nullptr);
        WriteProcessMemory(hProcess, (LPVOID)(local.pawnAddr + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashBangTime), &fZero, sizeof(fZero), nullptr);
        WriteProcessMemory(hProcess, (LPVOID)(local.pawnAddr + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_flFlashMaxAlpha), &fZero, sizeof(fZero), nullptr);
    }

    //空
    void AntiUtils::RemoveGrenade() {  
    }

    //空
    void AntiUtils::RemoveMolotov() {  
    }

} // namespace CS2Assist