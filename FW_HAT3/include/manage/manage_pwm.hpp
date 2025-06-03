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
    class PwmManager{
        // 基板情報
        const static uint8_t CONTROL_UNIT_MAX = 0xf;
        const static uint8_t CONTROL_UNIT_PORT = 0x4;
        const static uint8_t PORT_MAX = 0xf * 0x4;

        // コントロールはCAN側に基板実態を格納
        PwmServo::Serial serial_decoder;
        PwmServo::Can* control_model[CONTROL_UNIT_MAX];
        BoardLifeCycleW control_life[CONTROL_UNIT_MAX];
        // 返り値はシリアル側に基板実態を格納
        PwmFeedBack::Can can_decoder;
        PwmFeedBack::Serial* feedback_model[CONTROL_UNIT_MAX];
        BoardLifeCycle feedback_life[CONTROL_UNIT_MAX];
        
        // 送信用バッファ
        uint8_t send_frame[256];

    public: // インターフェイス関数群
        PwmManager(){
            // 基板実態の準備
            for (uint8_t child_idx = 0; child_idx < CONTROL_UNIT_MAX; child_idx++ ){
                control_model[child_idx] = new PwmServo::Can(child_idx);
                feedback_model[child_idx] = new PwmFeedBack::Serial();
                feedback_model[child_idx]->child_id = child_idx;
            }
        }

    public:
        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
        void update(uint16_t update_ms){
            // 制御指令送信
            for (uint8_t board_idx = 0; board_idx < CONTROL_UNIT_MAX; board_idx++){
                // ボードのライフサイクルがタイムオーバーしていたら更新処理を省略
                if (!control_life[board_idx].update(update_ms)) continue;
                
                // 送信処理
                GlobalInterface::can1.send(control_model[board_idx]->encode());
            }

            // フィードバック送信処理
            for (uint8_t board_idx = 0; board_idx < CONTROL_UNIT_MAX; board_idx++){
                // ボードのライフサイクルがタイムオーバーしていたら更新処理を省略
                if (!feedback_life[board_idx].update(update_ms)) continue;
                
                // フィードバックの生成処理
                uint8_t dlc = feedback_model[board_idx]->encode(send_frame);
                sendFrame(send_frame, dlc);
            }
        }
        /**
         * @brief CAN受信処理用関数
         * @return 該当する場合はtrueを返す
         */
        bool rcvCan(CanMessage& msg){
            // メッセージのデコード
            uint8_t child_id = can_decoder.decode(msg);
            // デコード結果が無効なら処理終了
            if (child_id == 0xff) return false;

            // フィードバックモデルのライフサイクル更新
            feedback_life[child_id].set();
            // デコード結果の反映
            for (uint8_t idx = 0; idx < CONTROL_UNIT_PORT; idx++){
                feedback_model[child_id]->fb_target[idx] = can_decoder.current_target[idx];
            }
            return true;
        }

        /**
         * @brief UART受信処理用関数
         * @return 該当する場合はtrueを返す
         */
        bool rcvCommand(uint8_t* msg){
            // コマンド種別を判別して無効なら処理終了
            if (msg[0] != PwmServo::Serial::SERIAL_ID) return false;
            
            // 有効ならデコード開始
            serial_decoder.decode(msg);
            // 受信したポート数だけ処理
            for (uint8_t idx = 0; idx < serial_decoder.port_num; idx++){
                uint8_t port = serial_decoder.port[idx];
                //　無効なポートは弾く
                if (port >= PORT_MAX) continue;
                
                // 制御インデックスの計算
                uint8_t ctrl_idx = port / CONTROL_UNIT_PORT;
                uint8_t port_idx = port % CONTROL_UNIT_PORT;
                // ライフサイクルの更新
                control_life[ctrl_idx].set();
                // 制御情報の格納
                control_model[ctrl_idx]->target[port_idx] = serial_decoder.pos[idx];
                control_model[ctrl_idx]->speed[port_idx] = serial_decoder.spd[idx];
            }
            return true;
        }
        /**
         * @brief Liveメッセージ受信関数
         * @note 情報を一元化してラズパイに送るため、デコードの二度手間を防ぐ目的
         */
        bool rcvLive(Live::Can& decode_msg){
            if (decode_msg.board_id < PwmServo::BASE_CAN_ID) return false;
            if (decode_msg.board_id > (PwmServo::BASE_CAN_ID + 0xF)) return false;
            control_life[decode_msg.board_id - PwmServo::BASE_CAN_ID].set(1);
            return true;
        }
    };
}