#pragma once
#include <stdint.h>
#include "serial_msg.hpp"

namespace Motor{
    /**
     * @brief Serialプロトコル
     */
    struct Serial{
        static const uint8_t SERIAL_ID = 10; // 基板一つあたりのポート数
        static const uint8_t PORT_MAX = 8; // メッセージ一つあたりのポート数
        static const uint8_t PORT_NUM = 2; // 基板一つあたりのポート数
        static const uint8_t PORT_BLOCK = 3; // 1ポート情報当たりのバイト数
        // ポート番号から、該当する基板IDを算出します。
        static uint8_t getBoardID(uint8_t port){
            return port / PORT_NUM;
        }
    public: // 設定値
        // メッセージに含むポート数
        uint8_t port_num = 0;
        // 制御電圧(0x7FF~-0x7FF)
        int16_t duty[PORT_MAX];
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
                SerialConvert::int16_2_array(duty[idx], &section[1]);
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
                duty[idx] = SerialConvert::array_2_int16(&section[1]);
            }
        }
    };
}

namespace MotorFeedback{
    /**
     * @brief Serialプロトコル
     */
    struct Serial{
    //     static const uint8_t SERIAL_ID = 11; // 基板一つあたりのポート数
    //     static const uint8_t PORT_MAX = 8; // メッセージ一つあたりのポート数
    //     static const uint8_t PORT_NUM = 2; // 基板一つあたりのポート数
    //     static const uint8_t PORT_BLOCK = 7; // 1ポート情報当たりのバイト数
    //     // ポート番号から、該当する基板IDを算出します。
    //     static uint8_t getBoardID(uint8_t port){
    //         return port / PORT_NUM;
    //     }
    // public: // 設定値
    //     // メッセージに含むポート数
    //     uint8_t port_num = 0;
    //     // 制御電圧(0x7FF~-0x7FF)
    //     int16_t output_duty[PORT_MAX];
    //     // 制御対象ポート
    //     uint8_t port[PORT_MAX];

    // public:
    //     /**
    //      * @brief シリアルコマンドのエンコード
    //      * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
    //      */
    //     uint8_t encode(uint8_t* frame){
    //         // ヘッダ情報の設定 
    //         frame[0] = SERIAL_ID;
    //         frame[1] = 0;

    //         // 制御情報の設定
    //         for (uint8_t idx = 0; idx < port_num; idx++){
    //             // ポートごとの情報の先頭を計算
    //             uint8_t* section = &frame[2 + idx * PORT_BLOCK];
    //             // ターゲットを設定
    //             section[0] = port[idx];
    //             section[1] = (uint8_t)((duty[idx] >> 8) & 0xFF);
    //             section[2] = (uint8_t)(duty[idx] & 0xFF);
    //             frame[1] += PORT_BLOCK;
    //         }
    //         // 全体長(DLC + 2)を返す
    //         return frame[1] + 2;
    //     }
        
    //     /**
    //      * @brief シリアルコマンドのデコード
    //      * @return 解析の成否
    //      */
    //     bool decode(uint8_t* frame){
    //         // ヘッダ情報の解析
    //         port_num = frame[1] / PORT_BLOCK;
    //         // 制御情報の取得
    //         for (uint8_t idx = 0; idx < port_num; idx++){
    //             // ポートごとの情報の先頭を計算
    //             uint8_t* section = &frame[2 + idx * PORT_BLOCK];
    //             port[idx] = section[0];
    //             uint16_t val = section[1];
    //             val = (val << 2) + section[2];
    //             duty[idx] = (int32_t)(val);
    //         }
    //    }
    };
}