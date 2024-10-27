#pragma once
#include "control/pwm_servo.h"
static const float add_range = 5.f / 100.f;
static float port_target = 0;
static float port_angle = 0;


void adjustUpdate(){
    if (buttonState(board.buttons[0]))
        port_angle += add_range;
    if (buttonState(board.buttons[1]))
        port_angle -= add_range;
    if (port_angle > 255.f) port_angle = 255.f;
    if (port_angle < 0.f) port_angle = 0.f;
    for (uint8_t i = 0; i < 8; i++)
        setServoPos(i, (uint8_t)port_angle);
}

void adjustRun(){
    while (1)
    {        
        adjustUpdate();
        updateServo();
        ledFlash(&board.leds[0], 10);  
        ledFlash(&board.leds[4], 10);
        HAL_Delay(10);
    }
    
}