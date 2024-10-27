#pragma once
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_uart.h>
#include <string.h>
#include "board/board.h"

/**=========================================================
 * LED初期化・操作
============================================================*/
Led LedInit(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = _gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
    Led io_struct;
    io_struct.port = _gpio_group;
    io_struct.pin = _gpio_pin;
    board.leds[board.led_len++] = io_struct;
    io_struct.counter = 0;
    return io_struct;
}
void ledOn(Led io_struct){
    HAL_GPIO_WritePin(io_struct.port, io_struct.pin, GPIO_PIN_SET);
}
void ledOff(Led io_struct){
    HAL_GPIO_WritePin(io_struct.port, io_struct.pin, GPIO_PIN_RESET);
}
void ledToggle(Led io_struct){
    HAL_GPIO_TogglePin(io_struct.port, io_struct.pin);
}
void ledFlash(Led* io_struct, uint16_t duration){
    io_struct->counter+=1;
    if (io_struct->counter > duration){
        io_struct->counter = 0;
        ledToggle(*io_struct);
    }
}