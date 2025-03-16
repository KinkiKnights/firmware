#pragma once
#include "can_msg.hpp"

/**
 * @note spd/free(4)[1-2/2], pos(4096)[1-1/2~0] 
 * @note CAN上に同系統基板は最大8台まで想定。0x08にて判別
 */
class ServoMessage{
public:
    static const uint16_t SERVO_BASE_ID = 0x200;
    static const uint16_t MASK_OFFSET_ID = 0x08;
private:
    static const uint16_t MASK_SPEED = 0xF0;
    static const uint16_t MASK_POSITION = 0x0FFF;

public:
    bool offset_port;
    uint16_t counter[4]; // 4000カウントまで(10000カウント/20ms)
    uint8_t speed[4];

    static uint8_t getChildNumber(CanMessage& msg){
        uint8_t id = (msg.id|(~MASK_OFFSET_ID)) - SERVO_BASE_ID;
        if (id < 8) return id;
        else return 0xff;
    }

    
    inline static bool isMe(CanMessage& msg, uint16_t& id){
        if (msg.id == id) return true;
        if (msg.id == (id + MASK_OFFSET_ID))return true;
        return false;
    }

    CanMessage encode(uint16_t child_id, uint8_t port = 0){
        CanMessage msg;
        // ID設定
        msg.id = SERVO_BASE_ID + child_id;
        if (offset_port) msg.id += MASK_OFFSET_ID;
        // DLC・ポート設定
        msg.dlc = 8;
        msg.port = port;
        uint8_t idx;
        for(uint8_t i = 0; i < 4; i++){
            idx = i*2;
            if (counter[i] > MASK_POSITION) counter[i] = MASK_POSITION;
            msg.data[idx] = counter[i] & 0xff;
            msg.data[idx+1] = (counter[i]>>8) & 0xf;
            msg.data[idx+1] += (speed[i]<<4) & MASK_SPEED;
        }
        return msg;
    }

    void decode(CanMessage& msg){
        offset_port = msg.id & MASK_OFFSET_ID;
        uint8_t idx;
        for(uint8_t i = 0; i < 4; i++){
            idx = i*2;
            counter[i] = msg.data[idx+1] &~MASK_SPEED;
            counter[i] = counter[i]<<8;
            counter[i] += msg.data[idx];
            speed[i] = msg.data[idx+1] >> 4;
        }
    }
};
