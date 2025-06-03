#pragma once
#include <stdint.h>

namespace GM6020{
    struct Serial{
        static const uint8_t SERIAL_ID = 15; // 識別ID
        static const uint8_t PORT_MAX = 8; // 最大送信ポート数
        static const uint8_t PORT_NUM = 4; // 基板一つあたりのポート数
        static const uint8_t PORT_BLOCK = 3; // 1ポート情報当たりのバイト数
        // ポート番号から、該当する基板IDを算出します。
        static uint8_t getBoardID(uint8_t port){
            return port / PORT_NUM;
        }
    public: // 設定値
        // メッセージに含むポート数
        uint8_t port_num = 0;
        // 制御角度
        int16_t target[PORT_MAX];
        // 制御対象ポート
        uint8_t port[PORT_MAX];

    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定 
            frame[0] = SERIAL_ID;
            frame[1] = 0;

            // 制御情報の設定
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * PORT_BLOCK];
                // ターゲットを設定
                section[0] = port[idx];
                section[1] = static_cast<uint8_t>(target[idx] >> 8);
                section[2] = static_cast<uint8_t>(target[idx]) & 0xFF;
                frame[1] += PORT_BLOCK;
            }
            // 全体長(DLC + 2)を返す
            return frame[1] + 2;
        }
        
        /**
         * @brief シリアルコマンドのデコード
         * @return 解析の成否
         */
        bool decode(uint8_t* frame){
            // ヘッダ情報の解析
            port_num = frame[1] / PORT_BLOCK;
            // 制御情報の取得
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * PORT_BLOCK];
                port[idx] = section[0];
                target[idx] = static_cast<int16_t>(section[2] | (section[1] << 8));
            }
            return true;
        }
    };
}
namespace GM6020FeedBack{
    struct Serial{
        static const uint8_t SERIAL_ID = 16; // 識別ID
        static const uint8_t PORT_MAX = 7; // 最大送信ポート数
        static const uint8_t PORT_NUM = 4; // 基板一つあたりのポート数
        static const uint8_t PORT_BLOCK = 3; // 1ポート情報当たりのバイト数
        // ポート番号から、該当する基板IDを算出します。
        static uint8_t getBoardID(uint8_t port){
            return port / PORT_NUM;
        }
    public: // 設定値
        // エンコード用ポート別有効フラグ
        bool port_enable[PORT_MAX];
        // メッセージに含むポート数
        uint8_t port_num = 0;
        // フィードバック対象ポート
        uint8_t port[PORT_MAX];
        uint16_t fb_position[PORT_MAX];
        uint16_t fb_speed[PORT_MAX];
        uint16_t fb_current[PORT_MAX];

    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定 
            frame[0] = SERIAL_ID;
            frame[1] = 0;

            // 制御情報の設定
            for (uint8_t idx = 0; idx < PORT_MAX; idx++){
                // 有効でないポートは除外
                if (!port_enable[idx]) continue;
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + frame[1]];
                // ターゲットを設定
                section[0] = idx;
                section[1] = static_cast<uint8_t>(fb_position[idx] >> 8);
                section[2] = static_cast<uint8_t>(fb_position[idx] & 0xFF);
                // section[3] = static_cast<uint8_t>(fb_speed[idx] >> 8);
                // section[4] = static_cast<uint8_t>(fb_speed[idx] & 0xFF);
                // section[5] = static_cast<uint8_t>(fb_current[idx] >> 8);
                // section[6] = static_cast<uint8_t>(fb_current[idx] & 0xFF);
                frame[1] += PORT_BLOCK;
            }
            // 全体長(DLC + 2)を返す
            return frame[1] + 2;
        }
        
        /**
         * @brief シリアルコマンドのデコード
         * @return 解析の成否
         */
        bool decode(uint8_t* frame){
            // ヘッダ情報の解析
            port_num = frame[1] / PORT_BLOCK;
            // 制御情報の取得
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * PORT_BLOCK];
                port[idx] = section[0];
                fb_position[idx] = static_cast<uint16_t>(section[2] | (section[1] << 8));
                // fb_speed[idx] = static_cast<uint16_t>(section[4] | (section[3] << 8));
                // fb_current[idx] = static_cast<uint16_t>(section[6] | (section[5] << 8));
            }
            return true;
        }
    };
}