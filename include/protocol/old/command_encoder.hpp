#pragma once
#include <stdint.h>

namespace Command
{
    struct EncoderFeedback{
        static const uint8_t ENCODER_FEEDBACK_CMD = 18;
        static const uint8_t ENCODER_NUM = 4;
        uint8_t child_id = 0;
        uint16_t pulse[ENCODER_NUM];
        uint8_t port_num = 2;
        uint8_t encode(uint8_t* frame){
            uint8_t dlc = 2 * port_num + 1; 
            frame[0] = ENCODER_FEEDBACK_CMD;
            frame[1] = dlc;
            frame[2] = child_id;
            uint8_t idx4;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx4 = idx * 4;
                frame[idx4 + 3] = pulse[idx] & 0xFF;
                frame[idx4 + 4] = pulse[idx] >> 8;
            }
            return dlc + 2;
        }
        void decode(uint8_t* frame){
            port_num = frame[1] / 2; // dlc抽出
            child_id = frame[2];
            uint8_t idx2;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx2 = idx * 2;
                pulse[idx] = frame[idx2 + 3];
                pulse[idx] += (uint16_t)(frame[idx2 + 4]) <<8;
            }
        }
    };
} // namespace Command
