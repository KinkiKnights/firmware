#pragma once
#include "byte_serializer.h"

typedef struct {
    bool is_ok;
    bool sw_feedback;
}EpbStatus;


void encodeEpbStatus(uint8_t* dlc, uint8_t* frame, EpbStatus* _this){
    *dlc = 2;
    frame[0] = _this->is_ok;
    frame[1] = _this->sw_feedback;
}

void decodeEpbStatus(uint8_t dlc, uint8_t* frame, EpbStatus* _this){
    _this->is_ok = frame[0];
    _this->sw_feedback = frame[1];
    return;
}

typedef struct {
    bool sw_safety;
}EpbComand;

void encodeEpbComand(uint8_t* dlc, uint8_t* frame, EpbComand* _this){
    *dlc = 1;
    frame[0] = _this->sw_safety;
}

void decodeEpbComand(uint8_t dlc, uint8_t* frame, EpbComand* _this){
    _this->sw_safety = frame[0];
    return;
}