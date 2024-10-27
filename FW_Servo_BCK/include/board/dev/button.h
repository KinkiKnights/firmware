#pragma once
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_uart.h>
#include <string.h>
#include "board/board.h"

/**=========================================================
 * Button初期化・状態取得
============================================================*/
Button ButtonInit(GPIO_TypeDef* _gpio_group, uint32_t _gpio_pin){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = _gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
    Button io_struct;
    io_struct.port = _gpio_group;
    io_struct.pin = _gpio_pin;
    board.buttons[board.button_len++] = io_struct;
    return io_struct;
}

uint8_t buttonState(Button io_struct){
    return 1 - HAL_GPIO_ReadPin(io_struct.port, io_struct.pin);
}