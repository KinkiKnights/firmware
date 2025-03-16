#pragma once
#include <stdint.h>

namespace Command
{
    struct MotorControl{
        static const uint8_t MOTOR_CTRL_CMD = 12;
        static const uint8_t DIR_MASK = 0b10000000;
        static const uint8_t MOTOR_NUM = 4;
        uint8_t child_id = 0;
        int32_t target[MOTOR_NUM]; //16ビット + Dir
        uint8_t ctrl[MOTOR_NUM];
        uint8_t port[MOTOR_NUM];
        uint8_t port_num = 0;
        uint8_t encode(uint8_t* frame){
            uint8_t dlc = 4 * port_num + 1; 
            frame[0] = MOTOR_CTRL_CMD;
            frame[1] = dlc;
            frame[2] = child_id;
            uint8_t idx4;
            for (uint8_t idx = 0; idx < port_num; idx++){
                uint16_t target_abs = (target[idx] < 0) ? -target[idx] : target[idx];
                idx4 = idx * 4;
                frame[idx4 + 3] = port[idx];
                frame[idx4 + 4] = (ctrl[idx] & ~DIR_MASK) + (target[idx] < 0) ? DIR_MASK : 0;
                frame[idx4 + 5] = target_abs & 0xFF;
                frame[idx4 + 6] = target_abs >> 8;
            }
            return dlc + 2;
        }
        void decode(uint8_t* frame){
            port_num = (frame[1] - 1) / 4; // dlc抽出
            child_id = frame[2];
            uint8_t idx4;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx4 = idx * 4;
                port[idx] = frame[idx4 + 3];
                ctrl[idx] = frame[idx4 + 4] & ~DIR_MASK;
                target[idx] = frame[idx4 + 5];
                target[idx] += (uint16_t)(frame[idx4 + 6]) <<8;
                target[idx] *= (DIR_MASK & frame[idx4 + 4]) ? -1 : 1;
            }
        }
    };
} // namespace Command
