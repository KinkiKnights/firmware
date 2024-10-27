#pragma once
#include <stm32f3xx_hal.h>
#include "clock.h"
#define CAN_BUFF_NUM 100

typedef struct{
    GPIO_TypeDef *port;
    uint32_t pin;
    uint16_t counter;
}Led;
typedef struct{
    GPIO_TypeDef *port;
    uint32_t pin;
}Button;
typedef struct{
    TIM_HandleTypeDef* tim;
    uint32_t channel;
}Pwm;
typedef struct 
{
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
}Can;

enum CanID{
    ID_EpbComand,
    ID_EpbStatus,
    ID_LiveRequest,
    ID_LiveResponse,
    ID_ServoCmd,
    ID_ServoFB,
    ID_LedCmd
};
typedef struct{
    uint16_t can_id;
    bool epb_safety;

    uint8_t button_len;
    uint8_t led_len;
    uint8_t tim_len;
    uint8_t pwm_len;
    Button buttons[20];
    Led leds[20];
    UART_HandleTypeDef debug;
    TIM_HandleTypeDef tims[10];
    Pwm pwms[20];
    CAN_HandleTypeDef can;

    uint16_t rcv_can_len;
    Can rcv_can_buff[CAN_BUFF_NUM];

}Board;
Board board;
