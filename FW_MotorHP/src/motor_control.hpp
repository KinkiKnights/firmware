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
        
        printf("Debug Duty->%f\n", duty);
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
        decoder.ctrl_mode[0] = Motor::DEF::FREE;
        decoder.ctrl_mode[1] = Motor::DEF::FREE;
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
            if (decoder.ctrl_mode[port] == Motor::DEF::FREE) {
                mtrs[port]->free();
                printf("Free\n");
            } else if (decoder.ctrl_mode[port] == Motor::DEF::DUTY){
                float duty = decoder.getDuty(port);
                mtrs[port]->sendDuty(duty);
            }
        }
        return;
    }

    void setControl(CanMessage& msg){
        wd_counter = 0;
        decoder.decode(msg);
    }
};

class MotorTest{
    Motor::Can encoder;
    float test_duty;
    int32_t cc = 500;
public:
    MotorTest(float _test_duty){
        test_duty = _test_duty;
    }

    void update(uint8_t child_id, CanMessage& msg){
        cc= (cc+1)% 1000;
        float out = test_duty * (cc - 500) / 500.f;
        encoder.ctrl_mode[0] = Motor::DEF::DUTY;
        encoder.ctrl_mode[1] = Motor::DEF::DUTY;
        encoder.setDuty(out, 0);
        encoder.setDuty(out,1);
        printf("Debug Duty->%f\n", -out);
        msg = encoder.encode(child_id);
    }

};
