#pragma once
#include "board/board.h"

static enum Board{
    UNKNOWN = 0,
    EPB = 1,
    ICS = 2,
    PWM = 3,
    LED = 4,
    MOTOR = 5
};

static Can msg_live_response;
static uint8_t random_send_per_10 = 10;
static uint8_t random_send_count;

uint16_t genToken(){
    const char* time_str = __TIME__;
    int seed = 0;
    for (int i = 0; i < 8; ++i) {
        if (time_str[i] >= '0' && time_str[i] <= '9') {
            seed = seed * 16 + (time_str[i] - '0');
        } else if (time_str[i] >= 'A' && time_str[i] <= 'F') {
            seed = seed * 16 + (time_str[i] - 'A' + 10);
        } else if (time_str[i] >= 'a' && time_str[i] <= 'f') {
            seed = seed * 16 + (time_str[i] - 'a' + 10);
        }
    }
    srand(seed);
    return (uint16_t)rand();
}

void initLive(){
    LiveResponse msg;
    msg.can_id = board.can_id;
    msg.board_type = EPB;
    msg.is_sim = false;
    msg.version = 0x01;
    msg.random_token = genToken();
    // エンコード
    encodeLiveResponse(&msg_live_response.dlc, msg_live_response.data, &msg);
    msg_live_response.id = CANID_LIVE_RESPONSE_ID;
    random_send_per_10 = genToken() % 10;
}

void updateLive(){
    if (random_send_count == random_send_per_10){
        sendCanMsg(&board.can,msg_live_response);
    }
    if (random_send_count <= random_send_per_10){
        random_send_count++;
    }
}

void rcvLive(Can *msg){
    if (msg->id != CANID_LIVE_REQUEST_ID) return;
    LiveRequest decoder;
    decodeLiveRequest(msg->dlc, msg->data, &decoder);
    if ((board.can_id >> 8) == decoder.target) {
        random_send_count = 0;
    }
    return;
}