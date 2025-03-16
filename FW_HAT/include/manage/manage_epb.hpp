#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "../../include/util/param.hpp"
#include "../../include/util/uart_send.hpp"
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
        uint8_t cmd[8];
        uint8_t hp;
        uint8_t count = 0;

    private:
        // 基板情報
        EPB::Can encoder_emg;
        EPBFeedBack::Can decoder_emg_status;
        EPB::Serial epb_cmd;
        EPBFeedBack::Serial epb_fb;

        // 基板内部状態
        const uint16_t EPB_TIMEOUT_MS = 1000;
        uint64_t epb_timeout_count_ms;
        bool is_safety = false;
        bool is_safety_fb = false;

    public: // インターフェイス関数群
        EpbManager(uint16_t child_id)
        : can_id(EPB::Param::CAN_BASE_ID)
        , can_child_id(0){}

        // CAN_ID確認
        static int8_t isMeCan(uint16_t id){
            if (id == EPB::Param::CAN_BASE_ID) return 0;
            if (id == EPBFeedBack::Param::CAN_BASE_ID) return 0;
            return -1;
        }

        // コマンド確認(受信用なので、CMDのみ)
        static int8_t isMeSerial(uint8_t *frames){
            uint8_t board_type = frames[0];
            if (board_type == EPBFeedBack::Param::SERIAL_ID) return 0;
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
            // if (!BaseManager::update(update_ms))return;

            // 非常停止のタイムアウト判定
            if (epb_timeout_count_ms > EPB_TIMEOUT_MS)
                is_safety = true;

            // CANの送信処理
            GlobalInterface::can1.send(encoder_emg.encode(is_safety));
            // Hat通信送信処理
            if (count++ < 10) return;
            count = 0;
            uint8_t frames[11];
            frames[0] = EPBFeedBack::Param::SERIAL_ID;
            frames[1] = 10;
            for (uint8_t idx = 0; idx < 7; idx++){
                frames[idx + 2] = cmd[idx];
            }
            frames[9] = is_safety_fb;
            frames[10] = hp;
            frames[11] = global_board->limit->getState();
            sendFrame(frames, 12);

            
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        bool rcvLiving(LivingMessage& msg){
            if(msg.can_id != EPB::Param::CAN_BASE_ID) return false;
            BaseManager::rcvLiving(msg);
            return true;
        }

        /**
         * @brief CAN受信処理用関数
         * @note 非常停止基板状態のフィードバックを受け取り、内部変数に保存
         */
        void rcvCan(CanMessage& msg){
            if(msg.dlc< 8) return;
            is_safety_fb = cmd[0] >> 7;
            hp = cmd[0] & 0b01111111;
            for (uint8_t idx = 0; idx < 8; idx++){
                cmd[idx] = msg.data[idx];
            }
        }

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            epb_cmd.decode(msg);
            is_safety = epb_cmd.is_safety;
            epb_timeout_count_ms = 0;
        }

    };

}