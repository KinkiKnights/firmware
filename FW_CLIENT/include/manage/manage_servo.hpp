#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "./manage_base.hpp"

namespace BoardManager
{
    /**
     * @brief サーボ基板の状態管理と通信管理を行う
     * @note 定期的に処理を実施し、通信を確認した基板に対してはデータ送信処理を行う。
     */
    class PwmManager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 8;
        const uint16_t can_id;
        const uint8_t can_child_id;

    private:
        // 基板情報
        ServoPwm::Can encoder0;
        ServoPwm::Can encoder1;
        ServoPwm::Serial cmd_encoder;

    public: // インターフェイス関数群
        PwmManager(uint16_t child_id)
        : can_id(ServoPwm::Param::CAN_BASE_ID + child_id)
        , can_child_id(child_id){
            encoder0.offset_port = false;
            encoder1.offset_port = true;
        }

        // CAN_ID確認
        static int8_t isBoardCanIDCan(uint16_t& id){
            uint16_t id_base = id & 0xFF0;
            if (id_base == ServoPwm::Param::CAN_BASE_ID) 
                return id % 0X10;
            return -1;
        }

        // コマンド確認
        static int8_t isBoardCanIDSerial(uint8_t *frames){
            uint8_t board_type = frames[0];
            uint8_t board_id = frames[2];
            if (board_type == ServoPwm::Param::SERIAL_ID) return board_id;
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
            // CANの送信処理
            // printf("update:%d\n", can_id);
            GlobalInterface::can1.send(encoder0.encode(can_child_id));
            GlobalInterface::can1.send(encoder1.encode(can_child_id));
            GlobalInterface::can2.send(encoder0.encode(can_child_id));
            GlobalInterface::can2.send(encoder1.encode(can_child_id));
            
            // Hat通信送信処理
            //@ ToDo
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        bool rcvLiving(LivingMessage& msg){
            printf("ID:%d = %d(MSG)", can_id, msg.can_id);
            if(msg.can_id != can_id) return false;
            BaseManager::rcvLiving(msg);
            return true;
        }

        /**
         * @brief CAN受信処理用関数
         * @note 無し(後日パワー供給実装予定)
         */
        void rcvCan(CanMessage& msg){
        }

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            cmd_encoder.decode(msg);
            for (uint8_t idx = 0; idx < cmd_encoder.port_num; idx++) {
                ServoPwm::Can *encoder = (cmd_encoder.port[idx] < 4) ? &encoder0 : &encoder1;
                const uint8_t port = cmd_encoder.port[idx] % 4;
                encoder->position[port] = cmd_encoder.pos[idx];
                encoder->speed[port] = cmd_encoder.spd[idx];
            }
            printf("\nservo pos:");
            for (uint8_t i = 0; i< 4; i++)
                printf("%d,", encoder0.position[i]);
            for (uint8_t i = 0; i< 4; i++)
                printf("%d,", encoder1.position[i]);
            printf("\n");
        
        }
    };
    
    
}