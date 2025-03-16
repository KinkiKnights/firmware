#pragma once
#include <stdint.h>

namespace Command
{
    struct Emergency{
        static const uint8_t EMERGENCY_CMD = 1;
        static const uint8_t EMERGENCY_DLC = 1;
        bool is_safety;
        uint8_t encode(uint8_t* frame){
            frame[0] = EMERGENCY_CMD;
            frame[1] = EMERGENCY_DLC;
            frame[2] = (is_safety)?1:0;
            return 3;
        }
        void decode(uint8_t* frame){
            is_safety = (frame[2] == 1);
        }
    };
    struct EmergencyFeedback{
        static const uint8_t EMERGENCY_FEEDBACK_CMD = 2;
        static const uint8_t EMERGENCY_FEEDBACK_DLC = 1;
        bool is_safety;
        uint8_t encode(uint8_t* frame){
            frame[0] = EMERGENCY_FEEDBACK_CMD;
            frame[1] = EMERGENCY_FEEDBACK_DLC;
            frame[2] = (is_safety)?1:0;
            return 3;
        }
        void decode(uint8_t* frame){
            is_safety = (frame[2] == 1);
        }
    };
} // namespace Command
