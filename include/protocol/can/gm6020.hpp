#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace GM6020{
    const uint16_t Control_CAN_ID[2] = {0x1FF, 0x2FF};
    const uint16_t FB_BASE_CAN_ID = 0x205;

    // 4ポートずつ
    struct Can{
        uint8_t child_id; // 0 or 1
    public:
        int16_t target[4]; // ±25000
        uint16_t fb_position[4];
        uint16_t fb_speed[4];
        uint16_t fb_current[4];
        
    public:
        inline CanMessage encode(){
            CanMessage msg;
            msg.id = Control_CAN_ID[child_id];
            msg.dlc = 8;
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::int16_2_array(target[port], &msg.data[port * 2]);
            }
            return msg;
        }

        inline uint8_t decode(CanMessage& msg){
            if (msg.dlc < 8) return 0xFF;
            if (msg.id < FB_BASE_CAN_ID || msg.id > FB_BASE_CAN_ID + 6) return 0xFF;
            uint8_t port = msg.id - FB_BASE_CAN_ID;
            fb_position[port] = CanCovert::array_2_int16(&msg.data[0]);
            fb_speed[port] = CanCovert::array_2_int16(&msg.data[2]);
            fb_current[port] = CanCovert::array_2_int16(&msg.data[4]);
            return port;
        }
        
    public:
        Can(uint8_t child)
        : child_id(child){
            for (uint8_t port = 0; port < 4; port++)
            {
                target[port] = 0;
                fb_position[port] = 0;
                fb_speed[port] = 0;
                fb_current[port] = 0;
            }
        }

        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id < FB_BASE_CAN_ID || msg.id > FB_BASE_CAN_ID + 6) return -1;
            uint8_t port = msg.id - FB_BASE_CAN_ID;
            return port;
        }
    };
}