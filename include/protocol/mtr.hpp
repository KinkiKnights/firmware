#pragma once
#include <stdint.h>
#include "../util/byte_conversion.hpp"
#include "../util/param.hpp"
#include "./can_msg.hpp"

namespace Motor{
    namespace DEF
    {
        enum{
            FREE = 0,
            DUTY = 1,
            POSITION = 2,
            SPEED = 3
        };

        constexpr uint32_t MAX_TARGET = 0xffffff;
        constexpr uint32_t DUTY_MAX = 1000;
    } // namespace name
    

    /**
     * @brief CANプロトコル
     * @note モータは基本2ポート.4ポート基板はCANIDが8オフセットしたID構成になっている
     */
    struct Can{
        static const uint16_t OFFSET_ID = 0x08;

    public: // ユーティリティ関数
        /**
         * @brief CANのIDから子ＩＤを算出する
         * @return 子ID。想定外子IDの場合は0xFFを返す。
         *  */ 
        static uint8_t getChildNumber(CanMessage& msg){
            uint8_t id = (msg.id|(~OFFSET_ID)) - Param::CAN_BASE_ID;
            if (id < Param::BOARD_NUM) return id;
            else return 0xff;
        }

        /**
         * @brief CAＮメッセージのIDが一致するかを確認する
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static bool isMe(CanMessage& msg, uint16_t id){
            if (msg.id == id) return true;
            if (msg.id == (id + OFFSET_ID))return true;
            return false;
        }
    public: // メッセージエンコード・デコード
        bool offset_port = false;
        int32_t target[4]; // 24Bitまで
        uint8_t ctrl_mode[4];

        /**
         * CANメッセージをエンコードする
         */
        CanMessage encode(uint16_t child_id, uint8_t port = 0){
            CanMessage msg;

            // ヘッダ情報設定
            msg.id = Param::CAN_BASE_ID + child_id;
            if (offset_port) msg.id += OFFSET_ID;
            msg.dlc = 8;
            msg.port = port;

            for(uint8_t port = 0; port < 2; port++){
                // フレームの設定
                Command::int24_plus_to_array(&msg.data[port*4], target[port], ctrl_mode[port]);
            }
            return msg;
        }

        void decode(CanMessage& msg){
            offset_port = ((msg.id & OFFSET_ID) > 0);
            for(uint8_t port = 0; port < 2; port++){
                Command::array_to_int24_plus(&msg.data[port*4], target[port], ctrl_mode[port]);
            }
        }
    public: // 応用関数
        void setDuty(float duty, uint8_t port){
            int32_t binary= static_cast<int32_t>(duty * DEF::MAX_TARGET);
            target[port] = binary;
        }

        float getDuty(uint8_t port){
                return 1.f * static_cast<float>(target[port]) / DEF::MAX_TARGET;
        }
    };

    /**
     * @brief Serialプロトコル
     */
    struct Serial{
    public: // 設定値
        // 対称のボードのＩＤ
        uint8_t child_id = 0;
        // メッセージに含むポート数
        uint8_t port_num = 0;
        // 制御目標
        int32_t target[Param::PORT_NUM];
        // 制御種別
        uint8_t ctrl[Param::PORT_NUM];
        // 制御対象ポート
        uint8_t port[Param::PORT_NUM];

    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定
            uint8_t dlc = 2 * port_num + 1; 
            frame[0] = Param::SERIAL_ID;
            frame[1] = dlc;
            frame[2] = child_id;
            // 送信ポート数のバリデーション
            if (port_num > Param::PORT_NUM) return 0;
            // 制御情報の設定
            for (uint8_t idx = 0; idx < port_num; idx++){

                uint8_t targets = static_cast<uint8_t>( abs(target[idx]) >> 17);
                if (target[idx] < 0) targets += 0x80;

                frame[idx * 2 + 3] = port[idx];
                frame[idx * 2 + 4] = targets;
            }
            return dlc + 2;
        }
        
        /**
         * @brief シリアルコマンドのデコード
         */
        void decode(uint8_t* frame){
            // ヘッダ情報の解析
            port_num = (frame[1] -1) / 2; // dlc抽出
            child_id = frame[2];

            // 送信ポート数のバリデーション
            if (port_num > Param::PORT_NUM){
                port_num = 0;
                return;
            }

            // 制御情報の取得
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t ports = frame[idx * 2 + 3];
                uint8_t targets = frame[idx * 2 + 4];
                int32_t target_int = static_cast<int32_t>(targets & 0x7F) << 17;
                if (targets > 0x7F)
                    target_int *= -1;
                target[idx] = target_int;
                port[idx] = ports;
                ctrl[idx] = 1;
            }
        }
    };
};