#pragma once
#define F4_CPU
#include "../../include/protocol/can_msg.hpp"
#include "../../include/protocol/emergency_msg.hpp"
#include "../../include/protocol/live_msg.hpp"
#include "../../include/protocol/emergency_msg.hpp"
#include "../../include/dev/F4/interface.hpp"
#include "../command/_command.hpp"
#include "./manage_base.hpp"

namespace BoardManager
{
        /**
     * @brief 非常停止基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class EpbManager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 1;
        const uint16_t can_id;
        const uint8_t can_child_id;

    private:
        // 基板情報
        EmergencyMessage encoder_emg;
        EmergencyStatusMessage decoder_emg_status;
        Command::Emergency epb_cmd;
        Command::EmergencyFeedback epb_fb_cmd;

        // 基板内部状態
        const uint16_t EPB_TIMEOUT_MS = 1000;
        uint64_t epb_timeout_count_ms;
        bool is_safety = false;
        bool is_safety_fb = false;

    public: // インターフェイス関数群
        EpbManager(uint16_t child_id)
        : can_id(EmergencyMessage::EMERGENCY_ID)
        , can_child_id(0){}

        // CAN_ID確認
        static int8_t isMeID(uint16_t id){
            if (id == EmergencyMessage::EMERGENCY_ID) return 0;
            return -1;
        }

        // コマンド確認
        static int8_t isMeCmd(uint8_t *frames){
            uint8_t board_type = frames[0];
            if (board_type == Command::Emergency::EMERGENCY_CMD) return 0;
            return -1;
        }

    public:
        /**
         * @brief 定期実行処理
         * @note タイムアウトを確認し、接続時はデータの更新と各種送信処理を実施
         */
        void update(uint16_t update_ms){
            // 未接続でも処理したいため、先に非常停止メッセージのタイムアウトを計算
            epb_timeout_count_ms += update_ms;
            // タイムアウト処理 未接続であれば処理終了
            if (!BaseManager::update(update_ms))return;

            // 非常停止のタイムアウト判定
            if (epb_timeout_count_ms > EPB_TIMEOUT_MS)
                is_safety = true;

            // CANの送信処理
            GlobalInterface::can1.send(encoder_emg.encode(is_safety));
            
            // Hat通信送信処理
            
            //@ ToDo
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        bool rcvLiving(LivingMessage& msg){
            printf("(%d)", msg.can_id);
            if(msg.can_id != EmergencyMessage::EMERGENCY_ID) return false;
            BaseManager::rcvLiving(msg);
            return true;
        }

        /**
         * @brief CAN受信処理用関数
         * @note 非常停止基板状態のフィードバックを受け取り、内部変数に保存
         */
        void rcvCan(CanMessage& msg){
            is_safety_fb = decoder_emg_status.decode(msg);
        }

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            epb_cmd.decode(msg);
            is_safety = epb_cmd.is_safety;
            epb_timeout_count_ms = 0;
        }

    };

}