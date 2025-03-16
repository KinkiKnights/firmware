#pragma once
#include <stdint.h>
#include "can_msg.hpp"

/**
 * @note 生存確認
 * @note CANID(2)/RUN_TIME(2)/Serial(2)/Error(2)
 */
namespace BOARD_TYPE
{
    enum {
        HAT,
        Safety,
        PWM_SERVO,
        MD
    };
} // namespace BOARD_TYPE

class LivingMessage{
public:
    static const uint16_t LIVE_ID = 0x002;
    uint16_t can_id;
    uint16_t run_time;
    uint16_t serial_id = 0;
    uint8_t status_flag;
    uint8_t loop_time;
private:
    void setWByte(uint8_t* first_ptr, uint16_t& val){
        first_ptr[0] = val & 0xFF; 
        first_ptr[1] = val >> 8; 
    }
    uint16_t getWByte(uint8_t* first_ptr){
        return first_ptr[0] + ((uint16_t)first_ptr[1] << 8);
    }

public:
    CanMessage encode(){
        CanMessage msg;
        // ID設定
        msg.id = LIVE_ID;
        msg.dlc = 8;
        setWByte(msg.data, can_id);
        setWByte(msg.data + 2, run_time);
        setWByte(msg.data + 4, serial_id);
        msg.data[6] = status_flag;
        msg.data[7] = loop_time;
        return msg;
    }

    bool decode(CanMessage& msg){
        if (msg.dlc < 8) return false;
        can_id = getWByte(msg.data);
        run_time = getWByte(msg.data + 2);
        serial_id = getWByte(msg.data + 4);
        status_flag = msg.data[6];
        loop_time = msg.data[7];
        return true;
    }

    bool getFlag(uint8_t bits){
        return ((status_flag >> bits)&0b1) == 1;
    }

    void setFlag(uint8_t bits, bool flag){
        uint8_t mask = 0b1 << bits;
        if (flag)
            status_flag |= mask;
        else
            status_flag &= ~mask;
        return;
    }
};

