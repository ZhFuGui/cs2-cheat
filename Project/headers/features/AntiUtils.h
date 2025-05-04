#ifndef CS2ASSIST_ANTI_UTILS_H
#define CS2ASSIST_ANTI_UTILS_H

#include <windows.h>
#include <string>
#include "../entity/EntityMgr.h"
namespace CS2Assist {

    class AntiUtils {
    public:
        AntiUtils(HANDLE processHandle, HMODULE clientModule, Entity& pLocalEntity);
        ~AntiUtils();

        // 移除烟雾效果
        void RemoveSmoke();

        // 移除闪光弹效果
        void RemoveFlash();

        // 移除手雷效果
        void RemoveGrenade();

        // 移除燃烧瓶效果
        void RemoveMolotov();

        // 获取实体地址（供内部使用）
        uint64_t GetBaseEntity(int index) const;

    private:
        HANDLE hProcess;
        uint64_t ClientModuleAddress;
        Entity& local;

        // 通用实体处理函数
        void ProcessEntities(void (*processFunc)(HANDLE, uint64_t));
    };

} // namespace CS2Assist

#endif // CS2ASSIST_ANTI_UTILS_H