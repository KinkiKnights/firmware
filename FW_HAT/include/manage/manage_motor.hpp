#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "./manage_base.hpp"
#include "../util/uart_send.hpp"

namespace BoardManager
{
    /**
     * @brief モータドライバ基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class MotorManager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 16;
        static const uint8_t PORT_NUM = 16;
        const uint16_t can_id;
        const uint8_t can_child_id;
        uint32_t ctrl_cmd_count_ms[PORT_NUM];
        uint8_t send_frame[256];
        uint8_t counter = 0;

    private:
        // 基板情報
        Motor::Can encoder;
        Motor::Serial decoder_cmd_ctrl;
        Encoder::Can encoder_enc;
        Encoder::Serial encoder_enc_feedback;

    public: // インターフェイス関数群
        MotorManager(uint16_t child_id)
        : can_id(Motor::Param::CAN_BASE_ID + child_id)
        , can_child_id(child_id){
            // タイムアウト時間を初期設定
            for(uint8_t i = 0; i < PORT_NUM; i++){
                ctrl_cmd_count_ms[i] = Motor::Param::CTRL_CMD_TIMEOUT_MS;
            }
            encoder_enc.position[0] = 0;
            encoder_enc.position[1] = 0;
        }

        /**
         * @brief CAN IDがモータドライバの物か判別する
         * @return 該当CAN IDだとchild_id、非該当なら-1を返す
         * @note ベース+マネージャ数以内にIDが収まっているかを判定する
         * @note 複数のIDがある場合はそれに対応する
         **/
        static int8_t isMeCan(uint16_t& id){
            uint16_t id_base = id & 0xFF0;
            if (id_base == Motor::Param::CAN_BASE_ID) 
                return id % 0x10;
            if (id_base == Motor::Param::CAN_BASE_ID + Motor::Param::CAN_ENCODER_OFFSET) 
                return id % 0x10;
            return -1;
        }
        
        /**
         * @brief コマンドのタイプがモータドライバか判別する
         * @return 該当する場合はchild_idを、非該当なら-1を返す
         * @note マネージャ数を超える場合は非該当判定
         */
        static int8_t isMeSerial(uint8_t *frames){
            uint8_t board_type = frames[0];
            uint8_t board_id = frames[2];
            if (board_type == Motor::Param::SERIAL_ID && board_id < MANAGER_NUM) return board_id;
            return -1;
        }

    public:
        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
        uint64_t count = 0;
        void update(uint16_t update_ms){
            // タイムアウト処理 未接続であれば処理終了
            if (!BaseManager::update(update_ms))return;
            
            // 各アクチュエータのコマンドタイムアウトを計算
            for (uint8_t idx = 0; idx < 2; idx++){
                ctrl_cmd_count_ms[idx] += update_ms;
                if (ctrl_cmd_count_ms[idx] >= Motor::Param::CTRL_CMD_TIMEOUT_MS){
                    encoder.ctrl_mode[idx] = Motor::DEF::FREE;
                // printf("               %d=>free\n", ctrl_cmd_count_ms[0]);
                }
            }
            
            // printf("Motor%d: %f=>%f %f=>%f \n",can_child_id, encoder.getDuty(0), encoder.ctrl_mode[0], encoder.getDuty(1), encoder.ctrl_mode[1]);

            // CANの送信処理
            GlobalInterface::can1.send(encoder.encode(can_child_id));
            
            // Hat通信送信処理 @ ToDo
            if(counter++ > 10){
                counter = 0;
                encoder_enc_feedback.port_num = 2;
                encoder_enc_feedback.child_id = can_child_id;
                encoder_enc_feedback.position[0] = encoder_enc.position[0];
                encoder_enc_feedback.position[1] = encoder_enc.position[1];
                uint8_t send_num = encoder_enc_feedback.encode(send_frame);
                sendFrame(send_frame, send_num);
            }
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        void rcvLiving(LivingMessage& msg){
            BaseManager::rcvLiving(msg);
        }

        // CANメッセージ受信処理
        void rcvCan(CanMessage& msg){
            uint16_t encoder_id = can_id + Motor::Param::CAN_ENCODER_OFFSET;
            if (msg.id != encoder_id) return;
            encoder_enc.decode(msg);
        }

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            if(msg[0] == CMD_ID::MTR_CMD){
                decoder_cmd_ctrl.decode(msg);
                for (uint8_t idx = 0; idx < decoder_cmd_ctrl.port_num; idx++){
                    // 受信メッセージを反映
                    uint8_t& port = decoder_cmd_ctrl.port[idx];
                    encoder.ctrl_mode[port] = decoder_cmd_ctrl.ctrl[idx];
                    encoder.target[port] = decoder_cmd_ctrl.target[idx];
                    // ポートのタイムアウト用カウントリセット
                    ctrl_cmd_count_ms[port] = 0;
                }
            }
        }
    };

}