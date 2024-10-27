#pragma once
#include "byte_serializer.h"
#define LED_PORT_NUM 2

typedef struct {
    uint8_t color1[LED_PORT_NUM];
    uint8_t color2[LED_PORT_NUM];
    uint8_t pattern[LED_PORT_NUM];
    uint8_t speed[LED_PORT_NUM];
}LedCommand2Port;

void encodeLedCommand2Port(uint8_t* dlc, uint8_t* frame, LedCommand2Port* _this){
    *dlc = LED_PORT_NUM * 2;
    for (uint8_t i = 0; i < LED_PORT_NUM ; i++){
        frame[i * 2 + 0] = ((uint8_t)(_this->color1[i]) << 4);
        frame[i * 2 + 0] += (uint8_t)(_this->color2[i]);
        frame[i * 2 + 1] = ((uint8_t)(_this->pattern[i]) << 4);
        frame[i * 2 + 1] += (uint8_t)(_this->speed[i]);
    }
}

void decodeLedCommand2Port(uint8_t dlc, uint8_t* frame, LedCommand2Port* _this){
    for (uint8_t i = 0; i < LED_PORT_NUM ; i++){
        _this->color1[i] = (uint8_t)(frame[i*2 + 0] >> 4);
        _this->color2[i] = (uint8_t)(frame[i*2 + 0] & 0xF);
        _this->pattern[i] = (uint8_t)(frame[i*2 + 1] >> 4);
        _this->speed[i] = (uint8_t)(frame[i*2 + 1] & 0xF);
    }
    return;
}