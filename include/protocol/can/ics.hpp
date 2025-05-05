#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace IcsServo{
    constexpr uint16_t BASE_CAN_ID = 0x110;
    struct Can{
        uint8_t child_id;
    public:
        uint16_t target[2];
        uint8_t speed[2];
        
    public:
        inline CanMessage encode(){
            CanMessage msg;
            msg.id = BASE_CAN_ID + child_id;
            msg.dlc = 6;
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::uint16_2_array(target[port], &msg.data[port*3]);
                msg.data[port*3 + 2] = speed[port];
            }
            return msg;
        }

        inline void decode(CanMessage& msg){
            if (msg.dlc < 8) return;
            for(uint8_t port = 0; port < 4; port++){
                target[port] = CanCovert::array_2_uint16(&msg.data[port*3]);
                speed[port] = msg.data[port*3 + 2];
            }
            return;
        }
        
    public:
        Can(uint8_t child)
        : child_id(child){
            for (uint8_t port = 0; port < 4; port++)
            {
                target[port] = 0;
                speed[port] = 0;
            }
        }

        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id >= BASE_CAN_ID && msg.id <= BASE_CAN_ID + 0xF)return msg.id - BASE_CAN_ID;
            return -1;
        }
    };
}