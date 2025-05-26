#pragma once
#include <stdint.h>

namespace Live{
    struct Serial{
        static const uint8_t SERIAL_ID = 2; // 識別ID
        static const uint8_t INFO_MAX = 20; // 最大送信ポート数
        static const uint8_t INFO_BLOCK = 2; // 1ポート情報当たりのバイト数

        public: // 設定値
        uint8_t board_num = 0; // 接続ボード数
        uint16_t board_id[INFO_MAX]; // 接続ボードのCAN_ID
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
            for (uint8_t idx = 0; idx < board_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * INFO_BLOCK];
                // ターゲットを設定
                section[0] = static_cast<uint8_t>(board_id[idx] >> 8);
                section[1] = static_cast<uint8_t>(board_id[idx] & 0xFF);
                frame[1] += INFO_BLOCK;
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
            board_num = frame[1] / INFO_BLOCK;
            // 制御情報の取得
            for (uint8_t idx = 0; idx < board_num; idx++){
                // ポートごとの情報の先頭を計算
                uint8_t* section = &frame[2 + idx * INFO_BLOCK];
                board_id[idx] = static_cast<uint16_t>(section[1] | (section[0] << 8));
            }
            return true;
        }
    };
}