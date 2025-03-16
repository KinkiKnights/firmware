#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

class BLDCMotor{
    Pwm *PWM_A;

public:
    void free(){
        PWM_A->setDuty(0.99f, 800);
    }

    void sendDuty(float duty){
        if (duty > 0.f){
            PWM_A->setDuty(1.f - duty, 800);
        } else {
            free();
        }
    }

    BLDCMotor(Pwm* _PWM_A)
    :PWM_A(_PWM_A){
        free();
    }
};

class MotorControl{
    const uint16_t CONTROL_TERM;
    BLDCMotor** mtrs;
    Bldc::Can decoder;
    uint16_t wd_counter = 0;

public:
    MotorControl(uint16_t _term_ms, BLDCMotor** _motors)
    : CONTROL_TERM(_term_ms)
    , mtrs(_motors){
        decoder.speed[0] = 0;
        decoder.speed[1] = 0;
    }

    void update(){
        // ウォッチドグリセット
        wd_counter += CONTROL_TERM;
        if (wd_counter > 1000){
            mtrs[0]->free();
            mtrs[1]->free();
            return;
        }
        // 通常アップデート
        for (uint8_t port = 0; port < 2; port++){
            float duty = decoder.getDuty(port);
            mtrs[port]->sendDuty(duty);
            // printf("Debug                       Duty->%f\n", duty);
        }
        return;
    }

    void setControl(CanMessage& msg){
        wd_counter = 0;
        decoder.decode(msg);
        printf("getCAN %d, %d\n", msg.data[0], msg.data[1]);

    }
};

class MotorTest{
    Bldc::Can encoder;
    Button** buttons;
    float test_duty;
public:
    MotorTest(Button** _buttons, float _test_duty){
        buttons = _buttons;
        test_duty = _test_duty;
    }

    void update(uint8_t child_id, CanMessage& msg){
        if(buttons[0]->getState()){
            test_duty = 1.f;
        }
        else if(buttons[1]->getState()){
            test_duty = 0.5;
        }else {
            test_duty = 0.1f;
        }
        encoder.setDuty(0.8,0);
        encoder.setDuty(0.1,1);
        msg = encoder.encode(child_id);
    }

};
