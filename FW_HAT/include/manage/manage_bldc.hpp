#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "./manage_base.hpp"

namespace BoardManager
{
    /**
     * @brief モータドライバ基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class BldcManager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 16;
        static const uint8_t PORT_NUM = 16;
        static const uint16_t CTRL_CMD_TIMEOUT_MS = 3000;
        const uint16_t can_id;
        const uint8_t can_child_id;
        uint32_t ctrl_cmd_count_ms[PORT_NUM];

    private:
        // 基板情報
        Bldc::Can encoder;
        Bldc::Serial decoder_cmd_ctrl;
        Encoder::Can enc_decoder;
        Encoder::Serial enc_feedback;

    public: // インターフェイス関数群
        BldcManager(uint16_t child_id)
        : can_id(Bldc::Param::CAN_BASE_ID + child_id)
        , can_child_id(child_id){
            // タイムアウト時間を初期設定
            for(uint8_t i = 0; i < PORT_NUM; i++){
                ctrl_cmd_count_ms[i] = CTRL_CMD_TIMEOUT_MS;
            }
        }

        /**
         * @brief CAN IDがモータドライバの物か判別する
         * @return 該当CAN IDだとchild_id、非該当なら-1を返す
         * @note ベース+マネージャ数以内にIDが収まっているかを判定する
         * @note 複数のIDがある場合はそれに対応する
         **/
        static int8_t isBoardCanIDCan(uint16_t& id){
            uint16_t id_base = id & 0xFF0;
            if (id_base == Bldc::Param::CAN_BASE_ID) 
                return id % 0x10;
            return -1;
        }
        
        /**
         * @brief コマンドのタイプがモータドライバか判別する
         * @return 該当する場合はchild_idを、非該当なら-1を返す
         * @note マネージャ数を超える場合は非該当判定
         */
        static int8_t isBoardCanIDSerial(uint8_t *frames){
            uint8_t board_type = frames[0];
            uint8_t board_id = frames[2];
            if (board_type == Bldc::Param::SERIAL_ID && board_id < MANAGER_NUM) return board_id;
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
            
            // 各アクチュエータのコマンドタイムアウトを計算
            for (uint8_t idx = 0; idx < 2; idx++){
                ctrl_cmd_count_ms[idx] += update_ms;
                if (ctrl_cmd_count_ms[idx] >= CTRL_CMD_TIMEOUT_MS){
                    encoder.setDuty(0.f, idx);
                }
                // encoder.speed[idx] = 3000;//.setDuty(0.3f, idx);
            }

            // CANの送信処理
            GlobalInterface::can1.send(encoder.encode(0));
            
            // Hat通信送信処理 @ ToDo
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
            if(msg[0] == Bldc::Param::SERIAL_ID){
                decoder_cmd_ctrl.decode(msg);
                for (uint8_t idx = 0; idx < decoder_cmd_ctrl.port_num; idx++){
                    // 受信メッセージを反映
                    uint8_t& port = decoder_cmd_ctrl.port[idx];
                    encoder.speed[port] = decoder_cmd_ctrl.speed[idx];            
                    // ポートのタイムアウト用カウントリセット
                    ctrl_cmd_count_ms[port] = 0;
                }
            }
        }
    };

}