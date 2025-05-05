#pragma once
#define F4_CPU
#include "../../include/protocol/can_msg.hpp"
#include "../../include/protocol/live_msg.hpp"
#include "../../include/protocol/motor_msg.hpp"
#include "../../include/dev/F4/interface.hpp"
#include "../command/_command.hpp"
#include "./manage_base.hpp"

namespace BoardManager
{
    /**
     * @brief モータドライバ基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class MotorManager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 16;
        static const uint16_t CTRL_CMD_TIMEOUT_MS = 1000;
        const uint16_t can_id;
        const uint8_t can_child_id;
        uint32_t ctrl_cmd_count_ms[2] = {0,0};

    private:
        // 基板情報
        MotorMessage encoder;
        Command::MotorControl decoder_cmd_ctrl;
        Command::MotorConfig decoder_cmd_config;
        Command::MotorConfigFeedback encoder_cmd_config;

    public: // インターフェイス関数群
        MotorManager(uint16_t child_id)
        : can_id(MotorMessage::MOTOR_BASE_ID + child_id)
        , can_child_id(child_id){}

        // CAN_ID確認
        static int8_t isBoardCanIDID(uint16_t& id){
            uint16_t child = id - MotorMessage::MOTOR_BASE_ID;
            if (0 <= child && child < MANAGER_NUM) return child;
            return -1;
        }
        
        // コマンド確認
        static int8_t isBoardCanIDCmd(uint8_t *frames){
            uint8_t board_type = frames[0];
            uint8_t board_id = frames[1];
            if (board_type == Command::MotorControl::MOTOR_CTRL_CMD) return board_id;
            if (board_type == Command::MotorConfig::MOTOR_CNFG_CMD) return board_id;
            return -1;
        }

    public:
        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
        void update(uint16_t update_ms){
            // タイムアウト処理 未接続であれば処理終了
            if (!BaseManager::update(update_ms))return;
            
            for (uint8_t idx = 0; idx < 2; idx++){
                ctrl_cmd_count_ms[idx] += update_ms;
                if (ctrl_cmd_count_ms[idx] >= CTRL_CMD_TIMEOUT_MS){
                    encoder.control_mode[idx] = MotorMessage::FREE;
                }
            }
            if (ctrl_cmd_count_ms)
            // CANの送信処理
            GlobalInterface::can1.send(encoder.encode(can_child_id));
            
            // Hat通信送信処理
            //@ ToDo
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        void rcvLiving(LivingMessage& msg){
            BaseManager::rcvLiving(msg);
        }

        // CANメッセージ受信処理
        void rcvCan(CanMessage& msg){}

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            if(msg[1] == Command::MotorControl::MOTOR_CTRL_CMD){
                decoder_cmd_ctrl.decode(msg);
                for (uint8_t idx = 0; idx < decoder_cmd_ctrl.port_num; idx++){
                    uint8_t& port = decoder_cmd_ctrl.port[idx];
                    encoder.control_mode[port] = decoder_cmd_ctrl.ctrl[idx];
                    encoder.target[port] = decoder_cmd_ctrl.target[idx];
                    ctrl_cmd_count_ms[port] = 0;
                }
            } else if (msg[1] == Command::MotorConfig::MOTOR_CNFG_CMD){
                decoder_cmd_config.decode(msg);
                // ToDo 設定値送信
            }
        }
    };

}