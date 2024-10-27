#pragma once
#include "byte_serializer.h"

typedef struct{
    uint16_t target;
}LiveRequest;

void encodeLiveRequest(uint8_t* dlc, uint8_t* frame, LiveRequest* _this){
    *dlc = 1;
    frame[0] = _this->target;
}

void decodeLiveRequest(uint8_t dlc, uint8_t* frame, LiveRequest* _this){
    _this->target = frame[0];
    return;
}

typedef struct{
    uint16_t can_id;
    bool is_sim;
    uint8_t board_type, version;
    uint16_t random_token;
}LiveResponse;

void encodeLiveResponse(uint8_t* dlc, uint8_t* frame, LiveResponse* _this){
    *dlc = 7;
    frame[0] = (uint8_t)(_this->can_id >> 8);
    frame[1] = (uint8_t)(_this->can_id & 0xFF);
    frame[2] = _this->is_sim ? 1 :0;
    frame[3] = _this->board_type;
    frame[4] = _this->version;
    frame[5] = (uint8_t)(_this->random_token >> 8);
    frame[6] = (uint8_t)(_this->random_token & 0xFF);
}

void decodeLiveResponse(uint8_t dlc, uint8_t* frame, LiveResponse* _this){
    _this->can_id = frame[0];
    _this->can_id = _this->can_id << 8;
    _this->can_id += frame[1];
    _this->is_sim = frame[2];
    _this->board_type = frame[3];
    _this->version = frame[4];
    _this->random_token = frame[5];
    _this->random_token = _this->random_token << 8;
    _this->random_token += frame[6];
    return;
}