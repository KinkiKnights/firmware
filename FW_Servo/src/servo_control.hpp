#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

/**
 * @brief PWMサーボのモデル制御を実施する
 */
class PwmServoModel{
public:
    // 制御ポート
    Pwm** pwms;
    static const uint8_t NUM = 8;
    const uint16_t POS_DUTY_MAX = 10000;
private:
    // 制御用モデル(count/MAXpwm)
    float pos_duty_current[NUM];
    // 制御目標値(count/MAXpwm)
    float pos_duty_target[NUM];
    // 制御速度
    uint8_t speed_index[NUM];
    // 制御速度テーブル(count/ms)
    float speed_table[16];

public:
    void update(uint16_t term_ms){
        for (uint8_t port = 0; port < 8; port++){
            // 最大速度計算
            float limit = speed_table[speed_index[port]] * term_ms;
            float diff = pos_duty_target[port] - pos_duty_current[port];
            // 差分と比較して更新速度を決定
            if (diff > limit)
                pos_duty_current[port] += limit;
            else if (-diff > limit)
                pos_duty_current[port] -= limit;
            else 
                pos_duty_current[port] = pos_duty_target[port];
            // カウンタピリオド設定
            pwms[port]->setPeriod(static_cast<uint16_t>(pos_duty_current[port]));
        }
    }

    ServoPwm::Can decoder;
    void setControl(CanMessage& msg){    
        // デコード処理
        decoder.decode(msg);
        // ポートオフセット計算
        uint8_t port_offset = 0;
        if (decoder.offset_port)
            port_offset = 4;
        // デコード結果格納
        uint8_t port_idx = 0;
        for (port_idx = 0; port_idx < 4; port_idx++){
            pos_duty_target[port_idx + port_offset] = decoder.position[port_idx] * 1.f;
            speed_index[port_idx + port_offset] = decoder.speed[port_idx];
            // printf(":   Get Duty %f(%d)",pos_duty_target[port_idx + port_offset], 1);
        }
    }

    PwmServoModel(Pwm **_pwms)
    : pwms(_pwms){
        //===========各ポート初期化================
        for (uint8_t idx = 0; idx < 8; idx++){
            pos_duty_current[idx] = 0.f;
            pos_duty_target[idx] = 0.f;
            speed_index[idx] = 0;
        }
        
        
        //===========制御速度テーブル生成(count/ms)================
        // キープ指示
        speed_table[0] = 0;
        // 速度制御用テーブル生成
        uint8_t calc_idx = 1;
        float speed_exp = 0.001;
        // 各桁3種類*5桁
        for(uint8_t expi = 0; expi < 5; expi++){
            speed_table[calc_idx++] = speed_exp * 1; 
            speed_table[calc_idx++] = speed_exp * 3; 
            speed_table[calc_idx++] = speed_exp * 6; 
            speed_exp *= 10.f;
        }
        
    }

    uint16_t getCounter(uint8_t port){
        return (uint16_t)pos_duty_target[port];
    }
    uint16_t getCounterCurrent(uint8_t port){
        return (uint16_t)pos_duty_current[port];
    }
};

/**
 * @brief PWMサーボのテスト用メッセージ生成
 */
class PwmServoTest{
    const uint16_t MAX_COUNT = 1000;
    const uint16_t MIN_COUNT = 500;
    const uint16_t PLS_DIFF = 5;

    ServoPwm::Can* encoders[2];
    Button** buttons;
    uint16_t target_count;

public:
    PwmServoTest(Button** _buttons){
        buttons = _buttons;
        encoders[0] = new ServoPwm::Can();
        encoders[1] = new ServoPwm::Can();
        encoders[0]->offset_port = false;
        encoders[1]->offset_port = true;
        for (uint8_t idx = 0; idx < 4; idx++){
            encoders[0]->speed[idx] = 0xF;
            encoders[1]->speed[idx] = 0xF;
        }
        target_count = MIN_COUNT;
    }

    void genMsg(uint16_t can_id, CanMessage& p1, CanMessage& p2){
        if(buttons[0]->getState())
            target_count += PLS_DIFF;
        if(buttons[1]->getState())
            target_count -= PLS_DIFF;
        if (target_count > MAX_COUNT) target_count = MAX_COUNT;
        if (target_count < MIN_COUNT) target_count = MIN_COUNT;
        // 値更新
        for (uint8_t idx = 0; idx < 4; idx++){
            if (idx < 2){
                encoders[0]->position[idx] = target_count;
                encoders[1]->position[idx] = target_count;
            }else{
                encoders[0]->position[idx] = (MAX_COUNT - target_count) + MIN_COUNT;
                encoders[1]->position[idx] = (MAX_COUNT - target_count) + MIN_COUNT;
            }
            encoders[0]->speed[idx] = 0xF;
            encoders[1]->speed[idx] = 0xF;
        }
        p1 = encoders[0]->encode(can_id);
        p2 = encoders[1]->encode(can_id);
    }

    uint16_t getCounter(){
        return target_count;
    }

};