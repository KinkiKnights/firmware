#pragma once
#include "can_msg.hpp"

/**
 * @note spd/free(4)[1-2/2], pos(4096)[1-1/2~0] 
 * @note CAN上に同系統基板は最大8台まで想定。0x08にて判別
 */
class MotorMessage{
public:
    static const uint16_t MOTOR_BASE_ID = 0x100;
    static const uint64_t MASK_TARGET = 10000;
private:
    static const uint16_t MASK_MODE = 0x7F;
    static const uint16_t MASK_PM = 0x80;
    static const uint16_t MASK_Byte = 0xFF;

public:
    uint8_t control_mode[2];
    int64_t target[2];// 3バイト長

    static uint8_t getChildNumber(CanMessage& msg){
        uint8_t id = msg.id - MOTOR_BASE_ID;
        if (id < 0x10) return id;
        return 0xff;
    }

    
    inline static bool isMe(CanMessage& msg, uint16_t& id){
        if (msg.id == id) return true;
        return false;
    }

    CanMessage encode(uint16_t child_id, uint8_t port = 0){
        CanMessage msg;
        // ID設定
        msg.id = MOTOR_BASE_ID + child_id;
        // シンプルモードの設定
        msg.dlc = 8;
        msg.port = port;
        // ターゲット値
        for (uint8_t i = 0; i < 2; i++){
            msg.data[i * 4] = MASK_MODE & control_mode[i];
            int64_t val = target[i];
            if (val < 0){
                msg.data[i * 4] += MASK_PM;
                val *= -1;
            }
            msg.data[i * 4 + 1] = val & MASK_Byte;
            msg.data[i * 4 + 2] = (val >> 8) & MASK_Byte;
            msg.data[i * 4 + 3] = (val >> 16) & MASK_Byte;
        }
        return msg;
    }

    void decode(CanMessage& msg){
        for (uint8_t i = 0; i < 2; i++){
            control_mode[i] = msg.data[i * 4] &  MASK_MODE;
            target[i] = msg.data[i * 4 + 1] + (msg.data[i * 4 + 2] << 8) + (msg.data[i * 4 + 3] << 16);
            if ((msg.data[i * 4] & MASK_PM) != 0)
                target[i] *= -1;
        }
    }

    enum{
        FREE = 0,
        DUTY = 1,
        POSITION = 2,
        SPEED = 3
    };

    
    void setDuty(float duty, uint8_t port){
        int32_t binary= int32_t(duty * MASK_TARGET);
        target[port] = binary;
    }

    float getDuty(uint8_t port){
        if (target[port] < 0) 
            return -1.f * (-target[port]) / MASK_TARGET;
        else
            return 1.f * (target[port]) / MASK_TARGET;
    }

};