#pragma once
#include "byte_serializer.h"
#define SERVO_PORT_NUM_CMD 2
#define SERVO_PORT_NUM_FB 8    

typedef struct {
        bool is_two;
        uint8_t port[SERVO_PORT_NUM_CMD], angle[SERVO_PORT_NUM_CMD];
        float speed[SERVO_PORT_NUM_CMD]; // (0~255) per ms
}ServoCommand2Port;

ServoCommand2Port newServoCommand2Port(){
    ServoCommand2Port msg;
    msg.is_two = true;
    return msg;
}

void encodeServoCommand2Port(uint8_t* dlc, uint8_t* frame, ServoCommand2Port* _this){
    *dlc = _this->is_two ? 8 : 4;
    for (uint8_t i = 0; i < SERVO_PORT_NUM_CMD ; i++){
        frame[i * 4 + 0] = _this->port[i];
        frame[i * 4 + 1] = _this->angle[i];
        // 送信時にはper 100msに変換
        serializeFloatHalf(_this->speed[i] / 100.f, frame + i * 4 + 2);
        // 対象ポートが一つの場合は一度で処理終了
        if (!_this->is_two) break;
    }
}

void decodeServoCommand2Port(uint8_t dlc, uint8_t* frame, ServoCommand2Port* _this){
    _this->is_two = (dlc > 4);
    for (uint8_t i = 0; i < SERVO_PORT_NUM_CMD ; i++){
        _this->port[i] = frame[i * 4 + 0];
        _this->angle[i] = frame[i * 4 + 1];
        // 送信時にはper 100msに変換
        _this->speed [i] = 100.f * deserializeFloatHalf(frame + i * 4 + 2);
        // 対象ポートが一つの場合は一度で処理終了
        if (!_this->is_two) break;
    }
}

typedef struct{
    uint8_t angle[SERVO_PORT_NUM_FB];
}ServoFeedback8Port;

void encodeServoFeedback8Port(uint8_t* dlc, uint8_t* frame, ServoFeedback8Port* _this){
    *dlc = 8;
    for (uint8_t i = 0; i < SERVO_PORT_NUM_FB ; i++){
        frame[i] = _this->angle[i];
    }
}

void decodeServoFeedback8Port(uint8_t dlc, uint8_t* frame, ServoFeedback8Port* _this){
    for (uint8_t i = 0; i < SERVO_PORT_NUM_FB ; i++){
        _this->angle[i] = frame[i];
    }
}