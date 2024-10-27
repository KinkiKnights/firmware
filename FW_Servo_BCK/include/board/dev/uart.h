#pragma once
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_uart.h>
#include <string.h>
#include <stdio.h>
#include "board/board.h"

void UART_Init(UART_HandleTypeDef* huart, USART_TypeDef* port){
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    huart->Instance = port;
    huart->Init.BaudRate = 19200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(huart);
    // printf関数のための初期化
    setbuf(stdout, NULL);
}

void uartPrint(UART_HandleTypeDef* huart, char* str){
    HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);
}

int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&board.debug,(uint8_t *)ptr,len,10);
  return len;
}