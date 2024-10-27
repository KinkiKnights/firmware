#pragma once
#include "board/board.h"

void idChangeRun(){
    uint8_t _id = 0;
    while (1){
        if (!buttonState(board.buttons[0]) && !buttonState(board.buttons[1]))
            break;
        ledFlash(&board.leds[3], 5);
        ledFlash(&board.leds[4], 5);
        HAL_Delay(100);
    }
    bool prev_button = false;
    while (1){
        if(buttonState(board.buttons[0]) && buttonState(board.buttons[1]))
            break;
        bool now_button = buttonState(board.buttons[0]);
        if (prev_button && !now_button) _id = (_id+1)%0x10;
        prev_button = now_button;
        HAL_Delay(10);
        ledFlash(&board.leds[3], 5);
        ledFlash(&board.leds[4], 5);
    }
    // ここで保存処理を入れる
    ledOff(board.leds[3]);
    ledOff(board.leds[4]);
    while (1){
        for (uint8_t i = 0; i < _id; i++){
            ledOn(board.leds[1]);
            HAL_Delay(200);
            ledOff(board.leds[1]);
            HAL_Delay(400);
        }
        HAL_Delay(5000);
    }
}