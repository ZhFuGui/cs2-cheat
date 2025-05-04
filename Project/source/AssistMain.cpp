/*
CS2Assist /
├── headers / # 头文件目录，简洁命名
│   ├── core / # 核心模块
│   │   ├── Config.h            # 配置（偏移量、键位等）
│   │   └── ProcessUtil.h       # 进程操作（PID、内存控制）
│   ├── entity / # 实体模块
│   │   └── EntityMgr.h         # 实体管理和数据结构（人物）
│   ├── features / # 功能模块
│   │   ├── GlowEsp.h           # 发光透视功能
│   │   ├── AimAssist.h         # 自瞄功能
│   │   └── AntiUtils.h			 # 防道具功能的头文件
│   ├── system / # 游戏系统管理模块（新增）
│   │   └── GameSystemMgr.h     # 游戏系统管理和数据结构
│   ├── utils / # 工具模块
│   │   ├── KeyInput.h          # 按键控制
│   │   └── MiscUtil.h          # 杂项工具（声音、一些辅助函数）
│   └── AssistMain.h            # 主程序接口

├── source / # 源代码目录
│   ├── core / # 核心模块实现
│   │   ├── Config.cpp          # 配置实现
│   │   └── ProcessUtil.cpp     # 进程操作实现
│   ├── entity / # 实体模块实现
│   │   └── EntityMgr.cpp       # 实体管理实现（含线程）
│   ├── features / # 功能模块实现
│   │   ├── GlowEsp.cpp         # 发光透视实现（单独线程）
│   │   ├── AimAssist.cpp       # 自瞄实现（单独线程）
│   │   └── AntiUtils.cpp		 # 防道具功能的实现
│   ├── system / # 游戏系统管理模块实现（新增）
│   │   └── GameSystemMgr.cpp   # 游戏系统管理实现
│   ├── utils / # 工具模块实现
│   │   ├── ConstsUtil.h        # 存放特征码和各种定义常数
│   │   ├── KeyInput.cpp        # 按键控制实现
│   │   └── MiscUtil.cpp        # 杂项工具实现
│   └── AssistMain.cpp          # 主程序实现
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
        std::cout << "无法找到 client.dll!" << std::endl;
        CloseHandle(hProcess);
        return 1;
    };

    //实例化游戏系统管理对象
    CS2Assist::GameSystem gameSystem;
    CS2Assist::GameSystemMgr gameSystemMgr(hProcess, ClientModule);

    //实例化实体管理对象
    CS2Assist::EntityMgr entityMgr(hProcess, ClientModule);
    CS2Assist::Entity entityList[64];CS2Assist::Entity local;
    entityMgr.StartEntityUpdateThread(entityList, local);

    //实例化自瞄对象
    CS2Assist::AimAssist aimAssist(hProcess, ClientModule, gameSystem, entityMgr, entityList, local);
    CS2Assist::TargetScope scope{ CS2Assist::TargetScope::TargetType::EnemiesOnly };
    aimAssist.StartAim(CS2Assist::TargetMode::VisibleEntity, scope, CS2Assist::AimControl::AimType::Memory);
    
    //std::cout << std::hex << local.pawnAddr << std::endl;
    
    //实例化防烟防闪对象
    CS2Assist::AntiUtils* g_AntiUtils = new CS2Assist::AntiUtils(hProcess, ClientModule, local);

    //实例化发光对象
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