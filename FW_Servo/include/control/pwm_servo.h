#pragma once
#include <math.h>
#include "board/board.h"
#include "board/dev/uart.h"
#include "board/dev/pwm.h"
#define PORT_NUM 8

static uint16_t PWM_COUNTER_RANGE = 10000;
static uint16_t MIN_RAD_COUNT = 250;
static uint16_t MAX_RAD_COUNT = 1200;
static float MIN_RAD_RANGE = 0.f;
static float MAX_RAD_RANGE = M_PI;
static float PROCESSING_INTERVAL_MS = 100;

/**
 * @brief カウンタの比較値を算出します。
 * @param rad 0~pi
*/
uint16_t dutyRange(float rad){
    return (uint16_t)(rad / M_PI * (MAX_RAD_COUNT - MIN_RAD_COUNT) + MIN_RAD_COUNT); 
}

/**
 * @brief サーボのモデル
*/
typedef struct {
    float speed_per_ms;
    float target;
    float current;
    bool is_init;
} ServoController;
ServoController servos[PORT_NUM];

void initServo(){
    for (uint8_t i = 0; i < PORT_NUM; i++){
        servos[i].is_init = false;
    }
}

/**
 * 速度制御のサーボモデルを更新
*/
void updateServoModel(int port){
    float diff = servos[port].target - servos[port].current;
    float max_diff = servos[port].speed_per_ms * PROCESSING_INTERVAL_MS;
    if ((diff < max_diff) && (diff > -max_diff)){
        servos[port].current += diff;
    } else if (diff > 0) {
        servos[port].current += max_diff;
    } else {
        servos[port].current -= max_diff;
    }
    // if (port ==0)printf("RADIAN port0: %d\n", (int)(servos[port].current * 80));
}



float calcRealUnit(uint8_t angle){
    return (MAX_RAD_RANGE - MIN_RAD_RANGE) * angle / 0xff + MIN_RAD_RANGE;
}

uint8_t calcComplessUnit(float angle){
    float cutoff =  (angle - MIN_RAD_RANGE) / (MAX_RAD_RANGE - MIN_RAD_RANGE) * 0xff;
    if (cutoff < 0.f) return 0;
    if (cutoff > 255) return 0xff;
    return (uint8_t)cutoff;
}

/**==============================================
 * Public相当関数
================================================*/
void updateServo(){
    for (int i = 0; i < PORT_NUM; i++) {
        // サーボモデルを更新
        updateServoModel(i);
        // PWM間隔の算出
        uint16_t compare = dutyRange(servos[i].current);
        if (i >= board.pwm_len){
            printf("Servo Port need initialize : %d\n", i);
            continue;
        }
        // // PWM設定
        if (servos[i].is_init)
            pwmSet(&board.pwms[i], compare);
        else
            pwmSet(&board.pwms[i], 0);
    }
}

// サーボの角度制御を指定します
void setServoPos(uint8_t port, uint8_t angle){
    float target = calcRealUnit(angle);
    servos[port].target = target;
    servos[port].current = target;
    servos[port].is_init = true;
}

// サーボの速度制御を指定します。
void setServoSpeed(uint8_t port, uint8_t angle, float speed){
    servos[port].target = calcRealUnit(angle);
    servos[port].speed_per_ms = speed;
    servos[port].is_init = true;
}

// 現在のサーボ角度を取得します。(255段階)
uint8_t getServoPosition(uint8_t port){
    return calcComplessUnit(servos[port].current);
}

void rcvServoCanCmd(Can* msg){
    if (msg->id != board.can_id) return;
    ServoCommand2Port decoder;
    decodeServoCommand2Port(msg->dlc, msg->data, &decoder);
    uint8_t contain_port = (decoder.is_two) ? 2 : 1;
    for (uint8_t i = 0; i < contain_port; i++) {
        uint8_t port = decoder.port[i];
        if (decoder.speed[i] == 0.f)
            setServoPos(port, decoder.angle[i]);
        else
            setServoSpeed(port, decoder.angle[i], decoder.speed[i]);
    }
}

