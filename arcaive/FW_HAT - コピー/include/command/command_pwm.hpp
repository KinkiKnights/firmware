#pragma once
#include <stdint.h>
#include "byte_conversion.hpp"

namespace Command
{
    struct PwmServo{
        static const uint8_t PWM_SERVO_CMD = 4;
        static const uint8_t PWM_NUM = 8;
        static const uint8_t FREE_SPD = 0xff;
        static const uint8_t POS_NONE_SPD = 0xfe;
        uint8_t child_id = 0;
        uint16_t pos[PWM_NUM];
        uint8_t spd[PWM_NUM];
        uint8_t port[PWM_NUM];
        uint8_t port_num = 0;
        uint8_t encode(uint8_t* frame){
            uint8_t dlc = 4 * port_num + 1; 
            frame[0] = PWM_SERVO_CMD;
            frame[1] = dlc;
            frame[2] = child_id;
            uint8_t idx4;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx4 = idx * 4;
                frame[idx4 + 3] = port[idx];
                frame[idx4 + 4] = spd[idx];
                frame[idx4 + 5] = pos[idx]>>8;
                frame[idx4 + 6] = pos[idx]&0xFF;
            }
            return dlc + 2;
        }
        void decode(uint8_t* frame){
            port_num = (frame[1] -1) / 4; // dlc抽出
            child_id = frame[2];
            printf("\nportnum = %d",port_num);
            uint8_t idx4;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx4 = idx * 4;
                port[idx] = frame[idx4 + 3];
                spd[idx] = frame[idx4 + 4];
                pos[idx] = frame[idx4 + 5];
                pos[idx] = (pos[idx] << 8) + frame[idx4 + 6];
                printf("\nport:%d spd:%d pos:%d", port[idx], spd[idx], pos[idx]);
            }
        }
    };
} // namespace Command
