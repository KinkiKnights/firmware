#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stm32f3xx_hal_can.h>
#include "utils/can/can_serializer.h"
#include "board/dev/uart.h"
#include "board/board.h"

/* CAN1 init function */
void CAN1_Init(CAN_HandleTypeDef *hcan)
{
    hcan->Instance = CAN;
    hcan->Init.Prescaler = 2;
    hcan->Init.Mode = CAN_MODE_NORMAL;
    hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan->Init.TimeSeg1 = CAN_BS1_13TQ;
    hcan->Init.TimeSeg2 = CAN_BS2_2TQ;
    hcan->Init.TimeTriggeredMode = DISABLE;
    hcan->Init.AutoBusOff = DISABLE;
    hcan->Init.AutoWakeUp = DISABLE;
    hcan->Init.AutoRetransmission = DISABLE;
    hcan->Init.ReceiveFifoLocked = DISABLE;
    hcan->Init.TransmitFifoPriority = DISABLE;
    
    if (HAL_CAN_Init(hcan) != HAL_OK)
    {
        // Error_Handler();
    }
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hcan->Instance == CAN)
    {

        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(CAN_RX0_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
    }
}

void CAN_Filter_Config(CAN_HandleTypeDef *hcan)
{
    CAN_FilterTypeDef can1_filter_init;

    can1_filter_init.FilterActivation = ENABLE;
    can1_filter_init.FilterBank = 0;
    can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
    can1_filter_init.FilterIdHigh = 0;
    can1_filter_init.FilterIdLow = 0;
    can1_filter_init.FilterMaskIdHigh = 0;
    can1_filter_init.FilterMaskIdLow = 0;
    can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
    can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;
    can1_filter_init.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(hcan, &can1_filter_init) != HAL_OK){}
}

uint16_t copyCanBuff(Can* base,  Can* target, uint16_t* len){
    // 処理中に新しいデータフレームを受信しないよう割り込みを停止
    __disable_irq();
    for (uint16_t i = 0; i < *len; i++){
        target[i] = base[i];
    }
    uint16_t prev_len = *len;
    *len = 0;
    __enable_irq();
    return prev_len;
}

void sendCanMsg(CAN_HandleTypeDef *can, Can msg){
    uint8_t tx_data[8];
    uint32_t last_TxMailBox;
    CAN_TxHeaderTypeDef TxHeader;
    if(0 < HAL_CAN_GetTxMailboxesFreeLevel(can)){
        ledFlash(&board.leds[2], 4);
        TxHeader.StdId = 0x20;
        TxHeader.RTR = CAN_RTR_DATA;
        TxHeader.IDE = CAN_ID_STD;
        TxHeader.DLC = 8;
        TxHeader.TransmitGlobalTime = DISABLE;
        for (int i = 0; i < msg.dlc;i++){
            tx_data[i] = msg.data[i];
        }
        HAL_CAN_AddTxMessage(can, &TxHeader, tx_data, &last_TxMailBox);

    } else {
        // printf("MailBox is full\n");
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];
    if (HAL_CAN_GetRxMessage(&board.can, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK){
        ledFlash(&board.leds[1], 4);
        if (board.rcv_can_len < CAN_BUFF_NUM){
            Can* buff = &board.rcv_can_buff[board.rcv_can_len++];
            buff->id = (uint16_t)RxHeader.StdId;
            buff->dlc = RxHeader.DLC;
            for (uint8_t i = 0; i < RxHeader.DLC; i++)
                buff->data[i] = RxData[i];
        }
        // 受信情報デバッグ
        if (buttonState(board.buttons[1]))
            printf("can_id->:%ld\n", RxHeader.StdId);
    }
}

void CAN_RX0_IRQHandler(void)
{
  HAL_CAN_IRQHandler(&board.can);
}
