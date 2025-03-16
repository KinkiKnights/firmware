#pragma once
#include <stdint.h>

namespace CMD_ID
{
    enum ID{
        EPB_CMD = 1,
        EPB_FDB = 2,
        MTR_CMD = 6,
        BLD_CMD = 8,
        ENC_CMD = 10
    };
} // namespace CMD_ID

namespace Hat{
    namespace Param{
        constexpr uint16_t CAN_BASE_ID = 0x10;
    }    
}

namespace ServoPwm{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 8;
        constexpr uint8_t PORT_NUM = 8;
        constexpr uint8_t PORT_NUM_MAX = 4;
        constexpr uint16_t CAN_BASE_ID = 0x140;
        constexpr uint8_t SERIAL_ID = 4;
    }    
} // namespace Param

namespace Motor{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 8;
        constexpr uint8_t PORT_NUM = 2;
        constexpr uint8_t PORT_NUM_MAX = 4;
        constexpr uint16_t CAN_BASE_ID = 0x100;
        constexpr uint16_t CAN_ENCODER_OFFSET = 0x10;
        constexpr uint8_t SERIAL_ID = 6;
        constexpr uint16_t CTRL_CMD_TIMEOUT_MS = 2000;
    }    
} // namespace Param

namespace Encoder{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 8;
        constexpr uint8_t PORT_NUM = 2;
        constexpr uint8_t PORT_NUM_MAX = 4;
        constexpr uint8_t SERIAL_ID = 10;
    }
} // namespace Paramnamespace Encoder{

namespace Bldc{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 8;
        constexpr uint8_t PORT_NUM = 2;
        constexpr uint16_t CAN_BASE_ID = 0x120;
        constexpr uint8_t SERIAL_ID = 12;
    }
} // namespace Param
namespace C610{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 8;
        constexpr uint8_t PORT_NUM = 1;
        constexpr uint16_t CAN_BASE_ID = 0x200;
        constexpr uint8_t SERIAL_ID = 14;
    }
} // namespace Param
namespace C610_FB{
    namespace Param{
        constexpr uint8_t PORT_NUM = 4;
        constexpr uint8_t SERIAL_ID = 15;
    }
} // namespace Param
namespace EPB{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 1;
        constexpr uint16_t CAN_BASE_ID = 0x0;
        constexpr uint8_t SERIAL_ID = 0;
    }
} // namespace Param
namespace EPBFeedBack{
    namespace Param{
        constexpr uint8_t BOARD_NUM = 1;
        constexpr uint16_t CAN_BASE_ID = 0x1;
        constexpr uint8_t SERIAL_ID = 1;
    }
} // namespace Param
