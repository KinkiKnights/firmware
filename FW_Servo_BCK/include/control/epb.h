#pragma once
#include "board/board.h"

static uint8_t wdgt_epb = 0;

void initEpb(){}

void updateEpb(){
    // 0.01秒想定
    if (wdgt_epb > 100 ){
        board.epb_safety = false;
    } else {
        wdgt_epb++;
    }
}

void rcvEpb(Can *msg){
    if (msg->id != CANID_EPB_STATUS) return;
    EpbStatus decoder;
    decodeEpbStatus(msg->dlc, msg->data, &decoder);
    board.epb_safety = decoder.is_ok;
    wdgt_epb = 0;
    return;
}