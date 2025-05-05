#pragma once
#include <stdint.h>
#include "../util/byte_conversion.hpp"
#include "../util/param.hpp"
#include "./can_msg.hpp"

namespace EPBFeedBack{
    
    /**
     * @brief CANプロトコル
     */
    struct Can{
    public: // ユーティリティ関数
        /**
         * @brief CANのIDから子ＩＤを算出する
         * @return 子ID。想定外子IDの場合は0xFFを返す。
         **/
        static uint8_t getChildNumber(CanMessage& msg){
            if (msg.id == Param::CAN_BASE_ID) return 0;
            else return 0xff;
        }

        /**
         * @brief CAＮメッセージのIDが一致するかを確認する
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static bool isBoardCanID(CanMessage& msg, uint16_t& id){
            if (msg.id == Param::CAN_BASE_ID) return true;
            return false;
        }
    public: // メッセージエンコード・デコード
        bool is_safety = false;

        /**
         * CANメッセージをエンコードする
         */
        CanMessage encode(uint8_t port = 0){
            CanMessage msg;

            // ヘッダ情報設定
            msg.id = Param::CAN_BASE_ID;
            msg.dlc = 1;
            msg.port = port;
            msg.data[0] = (is_safety) ? 1 : 0;
            return msg;
        }

        void decode(CanMessage& msg){
            is_safety = (msg.data[0] == 1);
        }
    };

    /**
     * @brief Serialプロトコル
     */
    struct Serial{
    public: // 設定値
        // 対称のボードのＩＤ
        bool is_safety = false;

    public:
        /**
         * @brief シリアルコマンドのエンコード
         * @note farme1のDLCは、フレーム2以降のフレーム数、帰り値は総フレーム数
         */
        uint8_t encode(uint8_t* frame){
            // ヘッダ情報の設定
            uint8_t dlc = 1; 
            frame[0] = Param::SERIAL_ID;
            frame[1] = dlc;
            frame[2] = (is_safety) ? 1 : 0;
            return dlc + 2;
        }
        
        /**
         * @brief シリアルコマンドのデコード
         */
        void decode(uint8_t* frame){
            is_safety = (frame[2] == 1);
        }
    };
};