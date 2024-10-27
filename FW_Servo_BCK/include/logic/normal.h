#include "logic/test.h"
#include "logic/id.h"

void sendServoFbMessage(){
    Can msg;
    ServoFeedback8Port encoder;
    for (uint8_t i = 0; i < PORT_NUM; i++ ){
        encoder.angle[i] = getServoPosition(i);    
    }
    encodeServoFeedback8Port(&msg.dlc, msg.data, &encoder);
    msg.id = board.can_id + CANID_PWM_FB_OFFSET;
    sendCanMsg(&board.can,msg);
    return;
}



void servoMainRun(){
    // 各種
    initEpb();
    initLive();
    Can transaction_can_buffer[CAN_BUFF_NUM];
    while(1){        
        /**==============================================
         * CANの受信処理
        =================================================*/
        uint16_t len = copyCanBuff(board.rcv_can_buff, transaction_can_buffer, &board.rcv_can_len);
        for (uint16_t i = 0; i < len; i++){
            // 受信したメッセージの処理
            rcvEpb(&transaction_can_buffer[i]);
            rcvLive(&transaction_can_buffer[i]);
            rcvServoCanCmd(&transaction_can_buffer[i]);
        }
        /**==============================================
         * 諸々更新
        =================================================*/  
        updateEpb();
        updateLive();
        updateServo();
        sendServoFbMessage();
        ledFlash(&board.leds[0], 10);  
        HAL_Delay(10);
    }
}