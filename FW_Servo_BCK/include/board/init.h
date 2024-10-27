#pragma once
#include "board/board.h"
#include "board/dev/led.h"
#include "board/dev/button.h"
#include "board/dev/pwm.h"
#include "board/dev/can.h"

void boardConfig(){
    // クロック起動
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    board.led_len = 0;
    board.button_len = 0;
    board.tim_len = 0;
    board.pwm_len = 0;
    board.rcv_can_len = 0;
    board.epb_safety = false;
    // LEDの初期化
    LedInit(GPIOA, GPIO_PIN_15);
    LedInit(GPIOB, GPIO_PIN_3);
    LedInit(GPIOB, GPIO_PIN_4);
    LedInit(GPIOB, GPIO_PIN_5);
    LedInit(GPIOB, GPIO_PIN_6);
    // ボタンの初期化
    ButtonInit(GPIOB , GPIO_PIN_7);
    ButtonInit(GPIOA , GPIO_PIN_8);
    // PWMサーボ用タイマの初期化
    TimInitPwm(TIM2);
    TimInitPwm(TIM3);
    // PWMの初期化
    PwmInit(&board.tims[0], TIM_CHANNEL_1, GPIOA, GPIO_PIN_0);
    PwmInit(&board.tims[0], TIM_CHANNEL_2, GPIOA, GPIO_PIN_1);
    PwmInit(&board.tims[0], TIM_CHANNEL_3, GPIOA, GPIO_PIN_2);
    PwmInit(&board.tims[0], TIM_CHANNEL_4, GPIOA, GPIO_PIN_3);
    PwmInit(&board.tims[1], TIM_CHANNEL_1, GPIOA, GPIO_PIN_10);
    PwmInit(&board.tims[1], TIM_CHANNEL_2, GPIOA, GPIO_PIN_12);
    PwmInit(&board.tims[1], TIM_CHANNEL_3, GPIOB, GPIO_PIN_0);
    PwmInit(&board.tims[1], TIM_CHANNEL_4, GPIOB, GPIO_PIN_1);
    // CAN関連
    UART_Init(&board.debug, USART1);
    CAN1_Init(&board.can);
    CAN_Filter_Config(&board.can);
    HAL_CAN_Start(&board.can);
    HAL_CAN_ActivateNotification(&board.can, CAN_IT_RX_FIFO0_MSG_PENDING);
    
}
