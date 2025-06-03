#pragma once
#define UART1DMA
#include "../../include/dev/F4/interface.hpp"
#include <stdint.h>

void sendFrame(uint8_t* frame,uint8_t num){
    uint8_t header[2] = {0xF0, 0xF0};
    uint64_t sum = 0;
    for (uint8_t i = 0; i < num; i++){
        sum += frame[i];
        // printf("%d,", frame[i]);
    }
    uint8_t checksum = sum % 0x100;
    
    GlobalInterface::dma_uart.send(header,2);
    GlobalInterface::dma_uart.send(frame,num);
    GlobalInterface::dma_uart.send(&checksum,1);
}