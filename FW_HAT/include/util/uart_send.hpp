#pragma once
#define UART1DMA
#include "../../include/dev/F4/interface.hpp"
#include <stdint.h>
uint8_t ___sendFrame[500];
uint8_t ___sendFrameNum = 0;
bool dma_uart_sending_cycle = false;
void updateSendStatus(){
    dma_uart_sending_cycle = dma_uart_is_sending;
}

void sendFrame(uint8_t* frame,uint8_t num){
    if (dma_uart_sending_cycle) {
        #ifdef LOG_SERIAL
        printf("SendTimeout");
        #endif
        return;
    }
    uint8_t* section = &(___sendFrame[___sendFrameNum]);
    section[0] = 0xF0;
    section[1] = 0xF0;
    uint64_t sum = 0;
    #ifdef LOG_SERIAL
    printf("(%d)=>", num);
    #endif
    for (uint8_t i = 0; i < num; i++){
        sum += frame[i];
        section[i+2] = frame[i];
    #ifdef LOG_SERIAL
        printf("%d,", frame[i]);
    #endif
    }
    uint8_t checksum = sum % 0x100;
    section[num + 2] = checksum;
    #ifdef LOG_SERIAL
    printf("%d::", checksum);
    #endif
    ___sendFrameNum += num + 3;
}

void flashFrame(){
    GlobalInterface::dma_uart.send(___sendFrame,___sendFrameNum);
    ___sendFrameNum = 0;
}