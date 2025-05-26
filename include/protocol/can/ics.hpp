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
            CanMessage msg(BASE_CAN_ID + child_id, 8);
            for(uint8_t port = 0; port < 2; port++){
                CanCovert::uint16_2_array(target[port], &msg.data[port*3]);
                msg.data[port*3 + 2] = speed[port];
            }
            return msg;
        }

        inline uint8_t decode(CanMessage& msg){
            if (msg.dlc < 6) return 0xFF;
            for(uint8_t port = 0; port < 2; port++){
                target[port] = CanCovert::array_2_uint16(&msg.data[port*3]);
                speed[port] = msg.data[port*3 + 2];
            }
            return child_id;
        }
        
    public:
        Can(uint8_t child)
        : child_id(child){
            for (uint8_t port = 0; port < 2; port++)
            {
                target[port] = 0;
                speed[port] = 0;
            }
        }
    };
}


namespace IcsFeedBack{
    constexpr uint16_t BASE_CAN_ID = 0x190;
    struct Can{
        uint8_t child_id;
    public:
        uint16_t current_target[4];
        
    public:
        inline CanMessage encode(){
            CanMessage msg(BASE_CAN_ID + child_id, 8);
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::uint16_2_array(current_target[port], &msg.data[port*2]);
            }
            return msg;
        }

        inline uint8_t decode(CanMessage& msg){
            if (msg.dlc < 8) return 0xFF;
            if ((msg.id & 0xFF0) != BASE_CAN_ID) return 0xFF;
            child_id = msg.id - BASE_CAN_ID;
            for(uint8_t port = 0; port < 4; port++){
                current_target[port] = CanCovert::array_2_uint16(&msg.data[port*2]);
            }
            return child_id;
        }
        
    public:
        Can(uint8_t child = 0)
        : child_id(child){
            for (uint8_t port = 0; port < 4; port++)
            {
                current_target[port] = 0;
            }
        }
    };
}