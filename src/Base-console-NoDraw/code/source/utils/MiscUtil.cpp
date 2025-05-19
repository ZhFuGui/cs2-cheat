#include "../../headers/utils/MiscUtil.h"
#include "../../headers/utils/ConstsUtil.h"
namespace CS2Assist {

    void Sleep_Ms(int milliseconds) noexcept {
        if (milliseconds < 0)return;
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    };

} // namespace CS2Assist