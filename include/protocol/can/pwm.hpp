#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace PwmServo{
    constexpr uint16_t BASE_CAN_ID = 0x100;
    struct Can{
        uint8_t child_id;
    public:
        uint16_t target[4]; 
        uint8_t speed[4]; 
        
    public:
        inline CanMessage encode(){
            CanMessage msg(BASE_CAN_ID + child_id, 8);
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::uint12_4_2_array(target[port], speed[port], &msg.data[port*2]);
            }
            return msg;
        }

        inline uint8_t decode(CanMessage& msg){
            if (msg.dlc < 8) return 0xFF;
            int16_t cid = msg.id - BASE_CAN_ID;
            child_id = cid;
            
            for(uint8_t port = 0; port < 4; port++){
                CanCovert::array_2_uint12_4(target[port], speed[port], &msg.data[port*2]);
            }
            return child_id;
        }
        
    public:
        Can(uint8_t child = 0)
        : child_id(child){
            for (uint8_t port = 0; port < 4; port++)
            {
                target[port] = 0;
                speed[port] = 0;
            }
        }
    };
}


namespace PwmFeedBack{
    constexpr uint16_t BASE_CAN_ID = 0x180;
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