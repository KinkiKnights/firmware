#pragma once
#include "control/pwm_servo.h"

static uint16_t sequence_counter = 10;

void pwmTestUpdate(){
    sequence_counter--;
    uint16_t c = sequence_counter;
    for (int i = 0; i < PORT_NUM; i++) {
        if (c > 1900)
            setServoPos(i, 0);
        else if (c > 1700)
            setServoPos(i, 255);
        else if (c > 1500)
            setServoPos(i, 122);
        else if (c > 1300)
            setServoSpeed(i, 255, 0.0005);
        else if (c > 800)
            setServoSpeed(i, 0, 0.00015);
        else if (c > 400)
            if (getServoPosition(i) > 225)
                setServoSpeed(i, 255, 0.00003);
            else if (getServoPosition(i) > 90)
                setServoSpeed(i, 255, 0.00015);
            else
                setServoSpeed(i, 255, 0.0005);

    }
    if (sequence_counter < 1)
        sequence_counter = 2000;
    return;
}

void pwmTestRun(){
    while (1)
    {        
        pwmTestUpdate();
        updateServo();
        ledFlash(&board.leds[0], 10);  
        ledFlash(&board.leds[4], 10);
        HAL_Delay(10);
    }
    
}