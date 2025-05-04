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

        // �Ƴ�����Ч��
        void RemoveSmoke();

        // �Ƴ����ⵯЧ��
        void RemoveFlash();

        // �Ƴ�����Ч��
        void RemoveGrenade();

        // �Ƴ�ȼ��ƿЧ��
        void RemoveMolotov();

        // ��ȡʵ���ַ�����ڲ�ʹ�ã�
        uint64_t GetBaseEntity(int index) const;

    private:
        HANDLE hProcess;
        uint64_t ClientModuleAddress;
        Entity& local;

        // ͨ��ʵ�崦����
        void ProcessEntities(void (*processFunc)(HANDLE, uint64_t));
    };

} // namespace CS2Assist

#endif // CS2ASSIST_ANTI_UTILS_H