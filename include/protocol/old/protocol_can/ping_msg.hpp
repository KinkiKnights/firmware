#pragma once
#include "can_msg.hpp"

/**
 * @note 非常停止
 */
class PingMessage{
public:
    static const uint16_t PING_ID = 0x002;
    uint16_t serial_id = 0;
    uint16_t boot_sec = 0;
    bool is_power = false;

public:
    CanMessage encode(uint16_t my_id){
        CanMessage msg;
        // ID設定
        msg.id = PING_ID;
        msg.dlc = 7;
        msg.data[0] = my_id & 0xFF;
        msg.data[1] = my_id >> 8;
        msg.data[2] = is_power ? 1:0;
        msg.data[3] = boot_sec & 0xFF;
        msg.data[4] = boot_sec >> 8;
        msg.data[5] = serial_id & 0xFF;
        msg.data[6] = serial_id >> 8;
        return msg;
    }

    uint16_t decode(CanMessage& msg){
        is_power = (msg.data[2] == 1);
        
        boot_sec = msg.data[4];
        boot_sec = boot_sec << 8;
        boot_sec = msg.data[3];

        serial_id = msg.data[6];
        serial_id = serial_id << 8;
        serial_id = msg.data[5];
        
        uint16_t id = msg.data[1];
        id = id << 8;
        return id + msg.data[0];
    }
};
