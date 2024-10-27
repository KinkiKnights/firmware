#include <cstdint> 
#include <cmath>
#include "define.hpp"

namespace MotorSimpleBoardPhase
{
    enum BoardPhase{
        NoConnect = 0,
        Critical = 1,
        EPBStop = 2,
        Enable = 3
    };
}

namespace MotorSimpleControlType
{
    enum ControlType{
        Stop = 0,
        Break = 1,
        Duty = 2
    };
}


struct MotorSimpleOutput{
    uint8_t control_type;
    float target = 0.f;
    MotorSimpleOutput(uint8_t ctrl, float t){
        control_type = ctrl;
        target = t;
    }
};

class MotorSimpleAxis
{
    bool reverse = false;
    bool accel_control = false;
    uint8_t control_type;
    float target_value[2] = {0.f, 0.f};
    float target_output = 0.f;

public: // 定期実行
    MotorSimpleAxis(){
        control_type = MotorSimpleControlType::Stop;
    }
    void update(uint16_t cycles, uint8_t& board_phase){
        // モータON以外は停止動作
        if (board_phase != MotorSimpleBoardPhase::Enable){
            control_type = MotorSimpleControlType::Stop;
            target_value[0] = 0.f;
        }

        // Duty制御計算
        if (control_type == MotorSimpleControlType::Duty){
            if (accel_control){
                float max_acc = cycles * target_value[1];
                if (max_acc >= std::abs(target_output - target_value[0])){
                    target_output = target_value[0];
                } else if (target_output < target_value[0]) {
                    target_output += max_acc;
                } else {
                    target_output -= max_acc;
                }
            } else {
                target_output = target_value[0];
            }
        }
        // それ以外は停止
        else {
                target_output = 0.f;
        }
    }

public: // プリミティブコマンド
    void setDuty(float duty){
        // バリデーション
        if (duty > 0.98) duty = 0.98;
        if (duty < -0.98) duty = -0.98;
        // 状態処理
        control_type = MotorSimpleControlType::Duty;
        target_value[0] = duty;
        accel_control = false;
    }
    void setDutyAcc(float duty, float max_duty_ms){
        // バリデーション
        if (duty > 0.98) duty = 0.98;
        if (duty < -0.98) duty = -0.98;
        // 状態処理
        control_type = MotorSimpleControlType::Duty;
        target_value[0] = duty;
        target_value[1] = max_duty_ms;
        accel_control = true;
    }
    void setBreak(){
        control_type = MotorSimpleControlType::Break;
        target_value[0] = 0.f;
    }
};

class MotorSimpleBoard{
private: // 固定変数
    static const uint16_t ERROR_NUM = 255;

private: // 基板管理
    const uint8_t board_type = BoardType::MotorSimple;
    const uint8_t child_id;
    uint8_t phase = MotorSimpleBoardPhase::NoConnect;
    bool error_array[ERROR_NUM];


public:
    MotorSimpleBoard(uint8_t _id)
    :child_id(_id){
        for (int idx = 0; idx < ERROR_NUM; idx++) {
            error_array[idx] = false;
        }
    }

    void setCanMsg(){

    }

    void setSpiMsg(){

    }
    
    void setUartMsg(){

    }

    void update(){

    }
};


// モジュールもしくはコマンドから叩かれるイメージ
// STOP時の状態管理は基板層側で実施