#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/can/can_msg.hpp"
#include "../../include/protocol/serial/serial_msg.hpp"
#include "../util/uart_send.hpp"
#include "./life_cycle.hpp"

namespace BoardManager
{
    /**
     * @brief サーボ基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class Gm6020Manager{
        // 基板情報
        // 一つの制御用CANメッセージに割り当てられる4モータを1制御単位とする
        const static uint8_t CONTROL_UNIT_PORT = 4; // 制御単位あたりのポート数
        const static uint8_t CONTROL_UNIT_MAX = 2; //制御単位の数
        const static uint8_t MOTOR_MAX = 7; //制御可能なモータ数

        // ボードの死活管理を通信帯域削減のために行う
        BoardLifeCycle control_life[CONTROL_UNIT_MAX]; // 制御単位ベース
        BoardLifeCycle feedback_life[MOTOR_MAX];// モーターベース

        // コピー処理削減のため、各送信用通信classを情報格納用のモデルとして使用する
        GM6020::Can* control_model[CONTROL_UNIT_MAX];
        GM6020FeedBack::Serial feedback_model;

        // 受信用通信クラス
        GM6020::Serial serial_decoder;
        GM6020::Can can_decoder;
        // 送信処理用一時バッファ
        uint8_t send_frame[256];

    public: // インターフェイス関数群
        Gm6020Manager(){
            // 制御単位モデルのインスタンス化
            for (uint8_t idx = 0; idx < CONTROL_UNIT_MAX; idx++ ){
                control_model[idx] = new GM6020::Can(idx);
            }

            // フィードバックモデルのポート番号初期化
            // ポートは1~7のため注意
            for (uint8_t idx = 0; idx < MOTOR_MAX; idx++ ){
                feedback_model.port[idx] = idx + 1;
            }
        }

    public:
        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
    void update(uint16_t update_ms){
        for (uint8_t idx = 0; idx < CONTROL_UNIT_MAX; idx++){
            // タイムアウト計算
            if (!control_life[idx].update(update_ms)) continue;

            //後輪のPID制御
            // 右前輪（port0）→ 右後輪（port1）
            {
                float target_front = control_model[idx]->target[0];  // port0
                float current_rear = feedback_model.fb_speed[1];     // port1

                static float integral_r = 0;
                static float prev_error_r = 0;

                float error_r = target_front - current_rear;
                integral_r += error_r * update_ms / 1000.0f;
                float derivative_r = (error_r - prev_error_r) / (update_ms / 1000.0f);
                prev_error_r = error_r;

                float Kp = 0.5, Ki = 0.01, Kd = 0.01;
                float output_r = Kp * error_r + Ki * integral_r + Kd * derivative_r;

                control_model[idx]->target[1] = output_r; // port1（右後輪）
         }

            // 左前輪（port2）→ 左後輪（port3）
            {
                float target_front = control_model[idx]->target[2];  // port2
                float current_rear = feedback_model.fb_speed[3];     // port3

                static float integral_l = 0;
                static float prev_error_l = 0;

                float error_l = target_front - current_rear;
                integral_l += error_l * update_ms / 1000.0f;
                float derivative_l = (error_l - prev_error_l) / (update_ms / 1000.0f);
                prev_error_l = error_l;

                float Kp = 0.5, Ki = 0.01, Kd = 0.01;
                float output_l = Kp * error_l + Ki * integral_l + Kd * derivative_l;

                control_model[idx]->target[3] = output_l; // port3（左後輪）
            }

            // 送信処理
            GlobalInterface::can1.send(control_model[idx]->encode());
        }
    }

            
            // ボードのライフサイクルが待もらえれているポートのみエンコード準備
            for (uint8_t port_idx = 0; port_idx < MOTOR_MAX;port_idx++){
                feedback_model.port_enable[port_idx] = control_life[port_idx].update(update_ms);
            }
            // サーボ角度のフィードバックをエンコードして送信
            uint8_t dlc = feedback_model.encode(send_frame);
            sendFrame(send_frame, dlc);
        }

        /**
         * @brief CAN受信処理用関数
         * @return 該当する場合はtrueを返す
         */
        bool rcvCan(CanMessage& msg){
            // メッセージのデコード
            // port_idxは0始まり
            uint8_t port_idx = can_decoder.decode(msg);
            // デコード結果が無効なら処理終了
            if (port_idx == 0xff) return false;

            // フィードバックモデルのライフサイクルの更新
            feedback_life[port_idx].set();
            // デコード結果の反映
            for (uint8_t idx = 0; idx < 4; idx++){
                // 格納ポートの計算と範囲外の終了処理
                uint8_t port = idx + port_idx * 4;
                if (port > 6) break;

                feedback_model.fb_position[port] = can_decoder.fb_position[idx];
                feedback_model.fb_speed[port] = can_decoder.fb_speed[idx];
                feedback_model.fb_current[port] = can_decoder.fb_current[idx];    
            }
            return true;
        }

        /**
         * @brief UART受信処理用関数
         * @return 該当する場合はtrueを返す
         */
        bool rcvCommand(uint8_t* msg){
            // コマンド種別を判別して無効なら処理終了
            if (msg[0] != GM6020::Serial::SERIAL_ID) return false;
            // 有効ならデコード開始
            serial_decoder.decode(msg);
            for (uint8_t idx = 0; idx < serial_decoder.port_num; idx++){
                uint8_t port = serial_decoder.port[idx];
                // 無効なポートは弾く
                if (port< 1 || port > MOTOR_MAX) continue;

                // 制御インデックスの計算
                uint8_t ctrl_idx = (port - 1) / CONTROL_UNIT_PORT;
                uint8_t port_idx = port - 1 - CONTROL_UNIT_PORT * ctrl_idx;
                // ライフサイクルの更新
                control_life[ctrl_idx].set();
                // 制御情報の格納
                control_model[ctrl_idx]->target[port_idx] = serial_decoder.target[idx];   
            }
            // コマンド処理完了を返す
            return true;
        }
    };    
}