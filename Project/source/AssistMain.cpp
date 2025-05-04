/*
CS2Assist /
������ headers / # ͷ�ļ�Ŀ¼���������
��   ������ core / # ����ģ��
��   ��   ������ Config.h            # ���ã�ƫ��������λ�ȣ�
��   ��   ������ ProcessUtil.h       # ���̲�����PID���ڴ���ƣ�
��   ������ entity / # ʵ��ģ��
��   ��   ������ EntityMgr.h         # ʵ���������ݽṹ�����
��   ������ features / # ����ģ��
��   ��   ������ GlowEsp.h           # ����͸�ӹ���
��   ��   ������ AimAssist.h         # ���鹦��
��   ��   ������ AntiUtils.h			 # �����߹��ܵ�ͷ�ļ�
��   ������ system / # ��Ϸϵͳ����ģ�飨������
��   ��   ������ GameSystemMgr.h     # ��Ϸϵͳ��������ݽṹ
��   ������ utils / # ����ģ��
��   ��   ������ KeyInput.h          # ��������
��   ��   ������ MiscUtil.h          # ����ߣ�������һЩ����������
��   ������ AssistMain.h            # ������ӿ�

������ source / # Դ����Ŀ¼
��   ������ core / # ����ģ��ʵ��
��   ��   ������ Config.cpp          # ����ʵ��
��   ��   ������ ProcessUtil.cpp     # ���̲���ʵ��
��   ������ entity / # ʵ��ģ��ʵ��
��   ��   ������ EntityMgr.cpp       # ʵ�����ʵ�֣����̣߳�
��   ������ features / # ����ģ��ʵ��
��   ��   ������ GlowEsp.cpp         # ����͸��ʵ�֣������̣߳�
��   ��   ������ AimAssist.cpp       # ����ʵ�֣������̣߳�
��   ��   ������ AntiUtils.cpp		 # �����߹��ܵ�ʵ��
��   ������ system / # ��Ϸϵͳ����ģ��ʵ�֣�������
��   ��   ������ GameSystemMgr.cpp   # ��Ϸϵͳ����ʵ��
��   ������ utils / # ����ģ��ʵ��
��   ��   ������ ConstsUtil.h        # ���������͸��ֶ��峣��
��   ��   ������ KeyInput.cpp        # ��������ʵ��
��   ��   ������ MiscUtil.cpp        # �����ʵ��
��   ������ AssistMain.cpp          # ������ʵ��
*/
#include <Windows.h>
#include <iostream>
#include "..\headers\AssistMain.h"
#include "..\output\offsets.hpp"
#include "..\output\buttons.hpp"
#include "..\output\client_dll.hpp"
#include "..\headers\system\GameSystemMgr.h"
#include "..\headers\core\ProcessUtil.h"
#include "..\headers\entity\EntityMgr.h"
#include "..\headers\utils\MiscUtil.h"
#include "..\headers\features\AimAssist.h"
#include "..\headers\features\GlowEsp.h"
#include "..\headers\features\AntiUtils.h"
#include "..\headers\AssistMain.h"

#include <thread>


int main() {

    SetConsoleOutputCP(CP_UTF8);

    DWORD Pid = CS2Assist::ProcessUtil::GetProcessPid(L"cs2.exe");

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Pid);

    HMODULE ClientModule = CS2Assist::ProcessUtil::GetModuleHandle(hProcess, "client.dll");


    if (!ClientModule) {
        std::cout << "�޷��ҵ� client.dll!" << std::endl;
        CloseHandle(hProcess);
        return 1;
    };

    //ʵ������Ϸϵͳ�������
    CS2Assist::GameSystem gameSystem;
    CS2Assist::GameSystemMgr gameSystemMgr(hProcess, ClientModule);

    //ʵ����ʵ��������
    CS2Assist::EntityMgr entityMgr(hProcess, ClientModule);
    CS2Assist::Entity entityList[64];CS2Assist::Entity local;
    entityMgr.StartEntityUpdateThread(entityList, local);

    //ʵ�����������
    CS2Assist::AimAssist aimAssist(hProcess, ClientModule, gameSystem, entityMgr, entityList, local);
    CS2Assist::TargetScope scope{ CS2Assist::TargetScope::TargetType::EnemiesOnly };
    aimAssist.StartAim(CS2Assist::TargetMode::VisibleEntity, scope, CS2Assist::AimControl::AimType::Memory);
    
    //std::cout << std::hex << local.pawnAddr << std::endl;
    
    //ʵ�������̷�������
    CS2Assist::AntiUtils* g_AntiUtils = new CS2Assist::AntiUtils(hProcess, ClientModule, local);

    //ʵ�����������
    CS2Assist::GlowEsp* g_GlowEsp = new CS2Assist::GlowEsp();


    while (1) {
        gameSystemMgr.Update(gameSystem);
        std::cout << gameSystem.sensitivity << std::endl;
        
            std::cout << std::dec << entityList[2].steamID << std::endl;
            std::cout << std::hex <<local.controllerAddr << std::endl;
        Sleep(2000);
        system("cls");
    }

	system("pause");
   
	return 0;
}