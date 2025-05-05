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
    class C610Manager:private BaseManager{
    public:
        static const uint8_t MANAGER_NUM = 1;
        static const uint8_t PORT_NUM = 4;
        static const uint16_t CTRL_CMD_TIMEOUT_MS = 3000;
        const uint16_t can_id;
        const uint8_t can_child_id;
        uint32_t ctrl_cmd_count_ms[PORT_NUM];
        uint16_t position[PORT_NUM];
        uint16_t speed[PORT_NUM];
        uint16_t torque[PORT_NUM];
        uint8_t send_frame[256];
        uint8_t last_port = 200;
        uint8_t counter = 0;

    private:
        // 基板情報
        C610::Can encoder;
        C610::Serial decoder_cmd_ctrl;
        C610_FB::Serial encoder_status;

    public: // インターフェイス関数群
        C610Manager(uint16_t child_id)
        : can_id(C610::Param::CAN_BASE_ID + child_id)
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
            if (id_base == C610::Param::CAN_BASE_ID) 
                return 0;
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
            // 管理オブジェクトは1なので子IDは固定で0
            if (board_type == C610::Param::SERIAL_ID && board_id < MANAGER_NUM) return 0;
            return -1;
        }

    public:
        /**
         * @brief 定期実行処理
         * @note 通信先が内製基板ではないのでタイムアウト処理は無し、接続時はデータの更新と各種送信処理を実施
         */
        void update(uint16_t update_ms){

            // 各アクチュエータのコマンドタイムアウトを計算
            for (uint8_t idx = 0; idx < 4; idx++){
                ctrl_cmd_count_ms[idx] += update_ms;
                if (ctrl_cmd_count_ms[idx] >= CTRL_CMD_TIMEOUT_MS){
                    encoder.speed[idx] = 0;
                }
            }

            // encoder.speed[0] =500;
            // CANの送信処理
            GlobalInterface::can1.send(encoder.encode(C610::Param::CAN_BASE_ID));
            
            // Hat通信送信処理 @ ToDo
            if(counter++ > 10){
                counter = 0;
                uint8_t send_num = encoder_status.encode(send_frame);
                sendFrame(send_frame, send_num);
            }
            
        }

        /**
         * @brief 生存確認メッセージ処理用関数
         * @note 基板状態を受け取り、接続情報を更新
         */
        void rcvLiving(LivingMessage& msg){
            // 無し
        }

        // CANメッセージ受信処理
        void rcvCan(CanMessage& msg){
            uint16_t id = msg.id - C610::Param::CAN_BASE_ID - 1;
            encoder_status.position[id] =  ((uint16_t)msg.data[0] << 8) + msg.data[1];
            encoder_status.speed[id] =  ((uint16_t)msg.data[2] << 8) + msg.data[3];
            encoder_status.torque[id] =  ((uint16_t)msg.data[4] << 8) + msg.data[5];
            last_port = id;
        }

        // UARTコマンド受信処理
        void rcvCommand(uint8_t* msg){
            if(msg[0] == C610::Param::SERIAL_ID){
                decoder_cmd_ctrl.decode(msg);
                for (uint8_t idx = 0; idx < decoder_cmd_ctrl.port_num; idx++){
                    // 受信メッセージを反映
                    uint8_t port = decoder_cmd_ctrl.port[idx];
                    printf("                                                          send CAN to C610!, %d, %d\n", port, decoder_cmd_ctrl.speed[idx]);
                    encoder.speed[0] = decoder_cmd_ctrl.speed[idx];
                    // ポートのタイムアウト用カウントリセット
                    ctrl_cmd_count_ms[0] = 0;
                }
            }
        }
    };

}