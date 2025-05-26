#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace Motor{
    constexpr uint16_t BASE_CAN_ID = 0x50;
    struct Can{
        uint8_t child_id;
    public:
        int16_t target[2];
        
    public:
        inline CanMessage encode(){
            CanMessage msg(BASE_CAN_ID + child_id, 4);
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::int16_2_array(target[port], &msg.data[port*2]);
            }
            return msg;
        }

        inline uint8_t decode(CanMessage& msg){
            if (msg.dlc != 4) return 0xFF;
            int16_t cid = msg.id - BASE_CAN_ID;
            child_id = cid;

            if (cid > 0xF || cid < 0) return 0xFF;
            for(uint8_t port = 0; port < 2; port++){
                target[port] = CanCovert::array_2_int16(&msg.data[port*2]);
            }
            return child_id;
        }
        
    public:
        Can(uint8_t child = 0)
        : child_id(child){
            for (uint8_t port = 0; port < 4; port++)
            {
                target[port] = 0;
            }
        }
    };
}