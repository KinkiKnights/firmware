#include <cstdint>

namespace CanMsg{
    /**
     * @brief CANのメッセージ形式です。
     */
    struct Frame
    {
        static const uint8_t FRAME_MAX = 8;
        uint16_t id;
        uint8_t dlc;
        uint8_t frame[FRAME_MAX];
    };
}
