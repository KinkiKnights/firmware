#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

class MotorPort{
    Pwm *PWM;
    Led *IN_A, *IN_B;
    const float MAX_DIFF = 0.005f;
    float current_duty = 0.f;

public:
    void free(){
        IN_B->off();
        IN_A->on();
        PWM->setDuty(0.f);
        current_duty = 0.f;
    }

    void sendDuty(float target_duty){
        
        float diff = target_duty - current_duty;
        if (diff > MAX_DIFF)
            current_duty += MAX_DIFF;
        if (diff < -MAX_DIFF)
            current_duty -= MAX_DIFF;
        else
            current_duty = target_duty;
        if (current_duty > 0.f){
            IN_B->off();
            IN_A->on();
            PWM->setDuty(current_duty);
        } else {
            IN_A->off();
            IN_B->on();
            PWM->setDuty(-current_duty);
        }
        printf("Debug                       Duty->%f\n", current_duty);
    }

    MotorPort(Pwm* _PWM, Led* _INA, Led* _INB)
    :PWM(_PWM), IN_A(_INA), IN_B(_INB){
        free();
    }
};

class MotorControl{
    const uint16_t CONTROL_TERM;
    MotorPort** mtrs;
    Motor::Can decoder;
    uint16_t wd_counter = 0;

    void updateDuty(){

    }

    void updateSpd(){
        
    }

    void updatePos(){
        
    }

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
        if (wd_counter > 1000){
            mtrs[0]->free();
            mtrs[1]->free();
            return;
        }
        // 通常アップデート
        for (uint8_t port = 0; port < 2; port++){
            if (decoder.ctrl_mode[port] == Motor::DEF::FREE) {
                mtrs[port]->free();
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
    Button** buttons;
    float test_duty;
    uint8_t cc = 0;
public:
    MotorTest(Button** _buttons, float _test_duty){
        buttons = _buttons;
        test_duty = _test_duty;
    }

    void update(uint8_t child_id, CanMessage& msg){
        cc= (cc+1)% 10;
        if(buttons[0]->getState()){
            encoder.ctrl_mode[0] = Motor::DEF::DUTY;
            encoder.ctrl_mode[1] = Motor::DEF::DUTY;
            encoder.setDuty(test_duty,0);
            encoder.setDuty(test_duty,1);
            printf("Debug Duty->%f\n", test_duty);
        } else if(buttons[1]->getState()){
            encoder.ctrl_mode[0] = Motor::DEF::DUTY;
            encoder.ctrl_mode[1] = Motor::DEF::DUTY;
            encoder.setDuty(-test_duty,0);
            encoder.setDuty(-test_duty,1);
            printf("Debug Duty->%f\n", -test_duty);
        } else {
            encoder.ctrl_mode[0] = Motor::DEF::FREE;
            encoder.ctrl_mode[1] = Motor::DEF::FREE;
            printf("Debug Free\n");
        }
        msg = encoder.encode(child_id);
    }

};
