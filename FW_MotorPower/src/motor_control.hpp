#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

class MotorPort{
    Pwm *PWM_A, *PWM_B;

public:
    void free(){
        PWM_A->setDuty(0.f);
        PWM_B->setDuty(0.f);
    }

    void sendDuty(float duty){
        if (duty > 0.f){
            PWM_B->setDuty(0.f);
            PWM_A->setDuty(duty);
        } else {
            PWM_A->setDuty(0.f);
            PWM_B->setDuty(-duty);
        }
    }

    MotorPort(Pwm* _PWM_A, Pwm* _PWM_B)
    :PWM_A(_PWM_A), PWM_B(_PWM_B){
        free();
    }
};

class MotorControl{
    const uint16_t CONTROL_TERM;
    MotorPort** mtrs;
    Motor::Can decoder;
    uint16_t wd_counter = 0;

public:
    MotorControl(uint16_t _term_ms, MotorPort** _motors)
    : CONTROL_TERM(_term_ms)
    , mtrs(_motors){
        decoder.target[0] = 0;
        decoder.target[1] = 0;
    }

    void update(){
        // ウォッチドグリセット
        wd_counter += CONTROL_TERM;
        if (wd_counter > 2000){
            mtrs[0]->free();
            mtrs[1]->free();
            return;
        }
        // 通常アップデート
        for (uint8_t port = 0; port < 2; port++){
            float duty = decoder.target[port] * 1.f / 0x7FFF;  
            mtrs[port]->sendDuty(duty);
        }
        return;
    }

    bool setControl(CanMessage& msg){
        if (0xFF == decoder.decode(msg)) return false;
        wd_counter = 0;
        return true;
    }
};

class MotorTest{
    Motor::Can encoder;
    Button** buttons;
    uint16_t test_out;
    uint8_t cc = 0;
public:
    MotorTest(Button** _buttons, float _test_duty, uint8_t _child_id)
    : encoder(_child_id){
        buttons = _buttons;
        test_out = static_cast<int16_t>(_test_duty * 0x7FFF);
    }

    void update(uint8_t child_id, CanMessage& msg){
        cc= (cc+1)% 10;
        float output = 0.f;
        if(buttons[0]->getState()){
            output = test_out;
        }else if(buttons[1]->getState()){
            output = -test_out;
        }

        encoder.target[0]= output;
        encoder.target[1]= output;
        msg = encoder.encode();
    }

};
