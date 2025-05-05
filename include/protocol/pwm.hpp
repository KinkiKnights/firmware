#pragma once
#include <stdint.h>
#include "../util/byte_conversion.hpp"
#include "../util/param.hpp"
#include "./can_msg.hpp"

namespace ServoPwm{
    /**
     * @brief CANプロトコル
     * @note サーボは上下4ポートずつでCANIDが8オフセットしたID構成になっている
     */
    struct Can{
        static const uint16_t OFFSET_ID = 0x08;
        static const uint16_t Max_SPEED = 0x0F;
        static const uint16_t Max_POSITION = 0x0FFF;

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
        inline static bool isBoardCanID(CanMessage& msg, uint16_t& id){
            if (msg.id == id) return true;
            if (msg.id == (id + OFFSET_ID))return true;
            return false;
        }
    public: // メッセージエンコード・デコード
        bool offset_port;
        uint16_t position[4]; // 4000カウントまで(10000カウント/20ms)
        uint8_t speed[4];

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

            for(uint8_t port = 0; port < 4; port++){
                // 送信値のバリデーション
                if (position[port] > Max_POSITION) position[port] = Max_POSITION;
                if (speed[port] > Max_SPEED) speed[port] = Max_SPEED;
                
                // フレームの設定
                uint8_t* section = &msg.data[port*2];
                section[0] = position[port] & 0xff;
                section[1] = ((position[port]>>8) & 0xF) + (speed[port]<<4);
            }
            return msg;
        }

        void decode(CanMessage& msg){
            offset_port = ((msg.id & OFFSET_ID) > 0);
            for(uint8_t port = 0; port < 4; port++){
                uint8_t* section = &msg.data[port*2];
                position[port] = section[1] & (Max_POSITION >> 8);
                position[port] = (position[port]<<8) + section[0];
                speed[port] = (section[1]>>4)& Max_SPEED;
            }
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
        // 制御角度
        uint16_t pos[Param::PORT_NUM];
        // 制御速度
        uint8_t spd[Param::PORT_NUM];
        // 制御対象ポート
        uint8_t port[Param::PORT_NUM];

    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定
            uint8_t dlc = 4 * port_num + 1; 
            frame[0] = Param::SERIAL_ID;
            frame[1] = dlc;
            frame[2] = child_id;
            // 送信ポート数のバリデーション
            if (port_num > Param::PORT_NUM) return 0;

            // 制御情報の設定
            uint8_t* section;
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                section = &frame[3 + idx * 4];
                // ターゲットを設定
                section[0] = port[idx];
                section[1] = spd[idx];
                section[2] = pos[idx] >> 8;
                section[3] = pos[idx] & 0xFF;
            }
            return dlc + 2;
        }
        
        /**
         * @brief シリアルコマンドのデコード
         */
        void decode(uint8_t* frame){
            // ヘッダ情報の解析
            port_num = (frame[1] -1) / 4; // dlc抽出
            child_id = frame[2];

            // 送信ポート数のバリデーション
            if (port_num > Param::PORT_NUM){
                port_num = 0;
                return;
            }

            // 制御情報の取得
            uint8_t* section;
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                section = &frame[3 + idx * 4];
                port[idx] = section[0];
                spd[idx] = section[1];
                pos[idx] = section[2];
                pos[idx] = (pos[idx] << 8) + section[3];
            }
        }
    };
}



