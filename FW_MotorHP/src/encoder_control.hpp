#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

class EncoderControl{
    EncoderTimer** encoders;
    Encoder::Can msg_encorder;
    uint16_t can_id;

public:
    EncoderControl(uint16_t _can_id, EncoderTimer** _encoder)
    : encoders(_encoder)
    , can_id(_can_id){}

    void update(){
        msg_encorder.position[0] = encoders[0]->getCounter();
        msg_encorder.position[1] = encoders[1]->getCounter();
        GlobalInterface::can1.send(msg_encorder.encode(can_id));
        return;
    }
};
