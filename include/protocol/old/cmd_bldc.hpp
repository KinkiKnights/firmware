#pragma once
#include <stdint.h>
#include "byte_conversion.hpp"

namespace Command
{
    struct BLDC{
        static const uint8_t PORT_NUM = 2;
    public: // パラメータ
        uint8_t child_id = 0;
        uint8_t port_num = 0;
        uint8_t spd[PORT_NUM];
        uint8_t port[PORT_NUM];
    
    public: // 処理関係        
        uint8_t encode(uint8_t* frame){
            uint8_t dlc = 2 * port_num + 1; 
            frame[0] = CMD_ID::BLD_CMD;
            frame[1] = dlc;
            frame[2] = child_id;
            uint8_t idx2;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx2 = idx * 2;
                frame[idx2 + 3] = port[idx];
                frame[idx2 + 4] = spd[idx];
            }
            return dlc + 2;
        }
        void decode(uint8_t* frame){
            port_num = (frame[1] -1) / 2; // dlc抽出
            child_id = frame[2];
            uint8_t idx2;
            for (uint8_t idx = 0; idx < port_num; idx++){
                idx2 = idx * 2;
                port[idx] = frame[idx2 + 3];
                spd[idx] = frame[idx2 + 4];
            }
        }
    };
} // namespace Command
