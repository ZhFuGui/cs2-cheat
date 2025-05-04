#include "../../headers/features/GlowEsp.h"
#include "../../output/offsets.hpp"
#include "../../output/client_dll.hpp"
#include <iostream>

namespace CS2Assist {

    //������Ϸ���⺯��Ӧ�õ�ָ��ʵ��ķ���
    void GlowEsp::ApplyGlow(HANDLE hProcess, uint16_t entityIndex, Entity* entityList, const Entity& local) {
        // ���� Index �ҵ���Ӧ��ʵ��
        const Entity* targetEntity = &entityList[entityIndex];

        // ���ʵ����Ч������ֵΪ 0 ���Ǳ�����ң�ֱ�ӷ���
        if (!targetEntity || !targetEntity->isValid || targetEntity->health <= 0 ||
            targetEntity->controllerAddr == local.controllerAddr) {
            return;
        }

        // ���㷢����ص��ڴ��ַ
        uint64_t glowColorOverride = targetEntity->pawnAddr +
            cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow +
            cs2_dumper::schemas::client_dll::CGlowProperty::m_glowColorOverride;
        uint64_t glowFunc = targetEntity->pawnAddr +
            cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_Glow +
            cs2_dumper::schemas::client_dll::CGlowProperty::m_bGlowing;

        // ���÷���״̬
        BOOL bGlowing = true;
        uint32_t color;

        // ��� isTarget Ϊ true����������������ʹ�� targetColor ����
        if (targetEntity->isTarget) {
            color = GlowConfig::targetColor.ToUInt32();
        }
        else {
            // �ж��Ƿ�Ϊ���Ѳ������Ƿ񷢹�
            bool isTeammate = targetEntity->teamId == local.teamId;
            bool shouldGlow = (isTeammate && GlowConfig::glowTeammates) ||
                (!isTeammate && GlowConfig::glowEnemies);
            if (!shouldGlow) return;

            // ���ݶ���������ɫ
            color = isTeammate ? GlowConfig::teammateColor.ToUInt32() : GlowConfig::enemyColor.ToUInt32();
        }

        // д���ڴ�
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(glowColorOverride),
            &color, sizeof(color), nullptr);
        WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(glowFunc),
            &bGlowing, sizeof(bGlowing), nullptr);
    }

    //ȫԱ����
    void GlowEsp::FunctionGlow(HANDLE hProcess, Entity* entityList, const Entity& local) {

        //ʹ����ʵ�巢��
        for (int i = 0; i < 64; ++i) {
            if (entityList[i].isValid) {
                ApplyGlow(hProcess, i, entityList, local);
            }
        }
        return;
    };

    //ȫԱ����
    void GlowEsp::BlueXrayGlow(HANDLE hProcess, HMODULE clientModule, BOOL isOn) {

        //���ҵ���ַ��Ȼ���жϴ���isOn���������true��ôд��nop������д��origin

        uint64_t blueXrayAddr = 0;
        DWORD32 patchResult = FindBlueXrayAddress(hProcess, clientModule, blueXrayAddr);

        if (!patchResult) {
            std::cerr << "Failed to find BlueXray address" << std::endl;
            GlowConfig::isActive = false;
            return;
        }

        // ȷ��Ҫд�������
        const BYTE* patchData = isOn ? Consts::SignCode::noppedPatch : Consts::SignCode::originalPatch;
        size_t patchSize = sizeof(Consts::SignCode::noppedPatch); // �������� Patch ����һ��

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

        // ���״̬
        if (isOn) {
            std::cout << "BlueXray enabled at " << std::hex << blueXrayAddr << std::endl;
        }
        else {
            std::cout << "BlueXray disabled at " << std::hex << blueXrayAddr << std::endl;
        }
    };

    //Ѱ��xray��ַ
    DWORD32 GlowEsp::FindBlueXrayAddress(HANDLE hProcess, HMODULE clientModule, uint64_t& blueXrayAddr) {

        // �����ַ���ҵ���ֱ�ӷ��� TRUE
        if (blueXrayAddr) {
            return TRUE;
        }

        // ���Ե�һ��ǩ��
        if (ProcessUtil::ScanSignature(hProcess, clientModule, Consts::SignCode::BlueXray, blueXrayAddr)) {
            return (DWORD32)2;
        }

        // ���Եڶ���ǩ��
        if (ProcessUtil::ScanSignature(hProcess, clientModule, Consts::SignCode::BlueXrayNopped, blueXrayAddr)) {
            return (DWORD32)3;
        }

        // ��ʧ�ܣ����� FALSE
        std::cerr << "Failed to locate BlueXray address with both signatures" << std::endl;
        return FALSE;
    };

} // namespace CS2Assist