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
        const static uint8_t CONTROL_UNIT_PORT = 4;  ///< 1制御単位あたりのポート数（最大4台のモータをまとめて制御）
        const static uint8_t CONTROL_UNIT_MAX = 2;   ///< 制御単位の最大数（基板数やCAN IDで分かれる）
        const static uint8_t MOTOR_MAX = 7;          ///< 制御可能なモータポートの最大数（1〜7）

        // ボードの死活管理（通信タイムアウト検出など）
        BoardLifeCycle control_life[CONTROL_UNIT_MAX]; ///< 制御単位ごとの生存判定
        BoardLifeCycle feedback_life[MOTOR_MAX];       ///< モータごとの生存判定

        // モデル格納用CAN通信クラス
        GM6020::Can* control_model[CONTROL_UNIT_MAX];  ///< CAN送信用の制御モデル（制御単位ごと）
        GM6020FeedBack::Serial feedback_model;         ///< UART送信用のフィードバックモデル（全ポート）

        // 受信処理クラス
        GM6020::Serial serial_decoder;  ///< UART受信デコーダ
        GM6020::Can can_decoder;        ///< CAN受信デコーダ

        // UART送信用バッファ
        uint8_t send_frame[256];

    public:
        /**
         * @brief コンストラクタ。制御モデルのインスタンス化およびフィードバックポート初期化を行う。
         */
        Gm6020Manager(){
            for (uint8_t idx = 0; idx < CONTROL_UNIT_MAX; idx++){
                control_model[idx] = new GM6020::Can(idx);
            }

            // UART送信用にポート番号（1~7）を設定
            for (uint8_t idx = 0; idx < MOTOR_MAX; idx++){
                feedback_model.port[idx] = idx + 1;
            }
        }

        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
        void update(uint16_t update_ms){
            for (uint8_t idx = 0; idx < CONTROL_UNIT_MAX; idx++){
                // タイムアウトチェック
                if (!control_life[idx].update(update_ms)) continue;
                {
                    float target_front_r = control_model[idx]->target[0];  // port0:右前
                    float current_rear_r = feedback_model.fb_speed[1];     // port1:右後

                    static float integral_r = 0, prev_error_r = 0;
                    float error_r = target_front_r - current_rear_r;
                    integral_r += error_r * (update_ms / 1000.0f);
                    float derivative_r = (error_r - prev_error_r) / (update_ms / 1000.0f);
                    prev_error_r = error_r;

                    float Kp = 0.5, Ki = 0.01, Kd = 0.01;  // ゲインは仮の値
                    float output_r = Kp * error_r + Ki * integral_r + Kd * derivative_r;

                    control_model[idx]->target[1] = output_r; // port1:右後輪（追従目標）
                }

                {
                    float target_front_l = control_model[idx]->target[2];  // port2:左前
                    float current_rear_l = feedback_model.fb_speed[3];     // port3:左後

                    static float integral_l = 0, prev_error_l = 0;
                    float error_l = target_front_l - current_rear_l;
                    integral_l += error_l * (update_ms / 1000.0f);
                    float derivative_l = (error_l - prev_error_l) / (update_ms / 1000.0f);
                    prev_error_l = error_l;

                    float Kp = 0.5, Ki = 0.01, Kd = 0.01;
                    float output_l = Kp * error_l + Ki * integral_l + Kd * derivative_l;

                    control_model[idx]->target[3] = output_l; // port3:左後輪（追従目標）
                }

                // CAN送信処理：各制御ユニットに対してコマンドを送信
                GlobalInterface::can1.send(control_model[idx]->encode());
            }

            // 通信が有効なポートのみをフィードバック送信対象とする
            for (uint8_t port_idx = 0; port_idx < MOTOR_MAX; port_idx++){
                feedback_model.port_enable[port_idx] = control_life[port_idx].update(update_ms);
            }

            // フィードバック情報（角度・速度・電流）をUARTで送信
            uint8_t dlc = feedback_model.encode(send_frame);
            sendFrame(send_frame, dlc);
        }

        /**
         * @brief CAN受信処理用関数
         * @return 該当する場合はtrueを返す
         */
        bool rcvCan(CanMessage& msg){
            uint8_t port_idx = can_decoder.decode(msg);
            if (port_idx == 0xff) return false;

            feedback_life[port_idx].set();

            for (uint8_t idx = 0; idx < 4; idx++){
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
            if (msg[0] != GM6020::Serial::SERIAL_ID) return false;

            serial_decoder.decode(msg);
            for (uint8_t idx = 0; idx < serial_decoder.port_num; idx++){
                uint8_t port = serial_decoder.port[idx];
                if (port < 1 || port > MOTOR_MAX) continue;

                uint8_t ctrl_idx = (port - 1) / CONTROL_UNIT_PORT;
                uint8_t port_idx = port - 1 - CONTROL_UNIT_PORT * ctrl_idx;

                control_life[ctrl_idx].set();
                control_model[ctrl_idx]->target[port_idx] = serial_decoder.target[idx];
            }
            return true;
        }
    };
}
