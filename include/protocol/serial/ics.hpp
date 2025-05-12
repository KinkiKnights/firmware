#pragma once
#include <stdint.h>

namespace IcsServo{
    /**
     * @brief Serialプロトコル
     */
    struct Serial{
        static const uint8_t SERIAL_ID = 1; // 基板一つあたりのポート数
        static const uint8_t PORT_MAX = 16; // 基板一つあたりのポート数
        static const uint8_t PORT_NUM = 4; // 基板一つあたりのポート数
        static const uint8_t PORT_BLOCK = 4; // 1ポート情報当たりのバイト数
        // ポート番号から、該当する基板IDを算出します。
        static uint8_t getBoardID(uint8_t port){
            return port / PORT_NUM;
        }
    public: // 設定値
        // メッセージに含むポート数
        uint8_t port_num = 0;
        // 制御角度
        uint16_t pos[PORT_MAX];
        // 制御速度
        uint8_t spd[PORT_MAX];
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
                section[1] = spd[idx];
                section[2] = pos[idx] >> 8;
                section[3] = pos[idx] & 0xFF;
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
                spd[idx] = section[1];
                pos[idx] = section[2];
                pos[idx] = (pos[idx] << 8) + section[3];
            }
        }
    };
}

namespace IcsServoFeedBack{
    struct Serial{
        static const uint8_t SERIAL_ID = 2; // 識別ID
        static const uint8_t PORT_MAX = 4; // 最大送信ポート数
        static const uint8_t PORT_NUM = 4; // 基板一つあたりのポート数
        static const uint8_t PORT_BLOCK = 2; // 1ポート情報当たりのバイト数
        // ポート番号から、該当する基板IDを算出します。
        static uint8_t getBoardID(uint8_t port){
            return port / PORT_NUM;
        }
    public: // 設定値
        // メッセージに含むポート数
        uint8_t port_num = PORT_MAX;
        // フィードバック対象ポート
        uint8_t child_id = 0;
        uint16_t fb_target[PORT_MAX];
    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定 
            frame[0] = SERIAL_ID;
            frame[1] = 1;
            frame[2] =  child_id;//基板子ID

            // 制御情報の設定
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * PORT_BLOCK];
                // ターゲットを設定
                section[0] = static_cast<uint8_t>(fb_target[idx] >> 8);
                section[1] = static_cast<uint8_t>(fb_target[idx] & 0xFF);
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
            port_num = (frame[1] - 1) / PORT_BLOCK;
            // 制御情報の取得
            child_id = frame[2];
            for (uint8_t idx = 0; idx < port_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[3 + idx * PORT_BLOCK];
                fb_target[idx] = static_cast<uint16_t>(section[1] | (section[0] << 8));
            }
        }
    };
}