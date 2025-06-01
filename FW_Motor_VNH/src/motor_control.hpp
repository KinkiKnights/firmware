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
        IN_A->off();
        PWM->setDuty(0.f);
        current_duty = 0.f;
    }

    void sendDuty(float target_duty){
        if (target_duty < 0.05f && target_duty > -0.05f){
            free();
            // printf("Be Free ");
            // printf(" out->%f\n", target_duty);
            return;
        }
        
        float diff = target_duty - current_duty;
        if (diff > MAX_DIFF)
            current_duty += MAX_DIFF;
        else if (diff < -MAX_DIFF)
            current_duty -= MAX_DIFF;
        else{
            current_duty = target_duty;
        }
        if (current_duty > 0.f){
            IN_B->off();
            IN_A->on();
            PWM->setDuty(current_duty);
        } else {
            IN_A->off();
            IN_B->on();
            PWM->setDuty(-current_duty);
        }
        // printf(" out:1->%f:%f", current_duty, diff);
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
            // mtrs[0]->free();
            // mtrs[1]->free();
            // return;
        }
        // 通常アップデート
        for (uint8_t port = 0; port < 2; port++){
            float duty = decoder.target[port] * 1.f / 0x7FFF;  
            
            printf(" out:%d", decoder.target[port]);
            mtrs[port]->sendDuty(duty);
        }
        return;
    }

    bool setControl(CanMessage& msg, bool is_test = false){
        if (0xFF == decoder.decode(msg)) return false;
        wd_counter = 0;
        printf("In:%d", decoder.target[1]);
        return true;
    }
};

class MotorTest{
    Motor::Can encoder;
    Button** buttons;
    float test_out;
    uint8_t cc = 0;
public:
    MotorTest(Button** _buttons, float _test_duty, uint8_t _child_id)
    : encoder(_child_id){
        buttons = _buttons;
        test_out = _test_duty;
        encoder.child_id = _child_id;
    }

    void update(uint8_t child_id, CanMessage& msg){
        cc= (cc+1)% 10;
        float output = 0.f;
        if(buttons[0]->getState()){
            output = test_out;
            printf("B1");
        }else if(buttons[1]->getState()){
            output = -test_out;
            printf("B2");
        }
        printf(" output(%d):%f\n", encoder.child_id,output);

        // printf("Duty->%h\n", output);
        encoder.target[0]= static_cast<int16_t>(output * 0x7FFF);
        encoder.target[1]= static_cast<int16_t>(output * 0x7FFF);
        msg = encoder.encode();

    }

};
