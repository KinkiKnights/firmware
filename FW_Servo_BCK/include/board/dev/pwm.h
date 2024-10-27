#pragma once
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_uart.h>
#include <string.h>
#include "board/board.h"

// #define PWM_PRESCALER 32 // PWM Servo
// #define PWM_PRESCALER 1 // LED TAPE
#define PWM_PRESCALER 31 // PWM Servo

void TimInitPwm(TIM_TypeDef* tim){
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_HandleTypeDef* htim = &(board.tims[board.tim_len++]);
    htim->Instance = tim;
    htim->Init.Prescaler = PWM_PRESCALER;
    htim->Init.Period = 9999;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(htim) != HAL_OK)Error_Handler();
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig) != HAL_OK)Error_Handler();
    if (HAL_TIM_PWM_Init(htim) != HAL_OK)Error_Handler();
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(htim, &sMasterConfig) != HAL_OK)Error_Handler();
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)Error_Handler();
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)Error_Handler();
}

void PwmInit(TIM_HandleTypeDef* htim, uint32_t channel, GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
    // 基板オブジェクトの生成
    uint8_t pwm_index = board.pwm_len++;
    board.pwms[pwm_index].tim = htim;
    board.pwms[pwm_index].channel = channel;
    // GPIOの設定
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = _gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // 別タイマ使用時は追記の事
    if (htim->Instance == TIM2){
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    } else {
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    }
    HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
    HAL_TIM_PWM_Start(htim, channel);
}

void pwmSet(Pwm* pwm, uint16_t on_per_peroiod){
    __HAL_TIM_SET_COMPARE(pwm->tim, pwm->channel, on_per_peroiod);
}