#pragma once
#include <stdint.h>

namespace Command
{
    struct Emergency{
        static const uint8_t EMERGENCY_CMD = 1;
        bool is_safety;
        uint8_t encode(uint8_t* frame){
            frame[0] = EMERGENCY_CMD;
            frame[1] = (is_safety)?1:0;
            return 1;
        }
        void decode(uint8_t* frame){
            is_safety = (frame[1] == 1);
        }
    };
} // namespace Command
