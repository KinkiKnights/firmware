#pragma once
#include "can_msg.hpp"

/**
 * @note 非常停止
 */
class EmergencyMessage{
public:
    static const uint16_t EMERGENCY_ID = 0x000;

public:
    CanMessage encode(bool is_safety){
        CanMessage msg;
        // ID設定
        msg.id = EMERGENCY_ID;
        msg.dlc = 1;
        msg.data[0] = is_safety ? 1:0;
        return msg;
    }

    bool decode(CanMessage& msg){
        if (msg.dlc < 1) return false;
        return (msg.data[0] == 1);
    }
};

class EmergencyStatusMessage{
    static const uint16_t EMERGENCY_ID = 0x001;

public:
    CanMessage encode(bool is_safety){
        CanMessage msg;
        // ID設定
        msg.id = EMERGENCY_ID;
        msg.dlc = 1;
        msg.data[0] = is_safety ? 1:0;
        return msg;
    }

    bool decode(CanMessage& msg){
        if (msg.dlc < 1) return false;
        return (msg.data[0] == 1);
    }
};
