#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace Motor{
    enum {
        DUTY = 0x1,
        SPEED = 0x2,
        POSITION = 0x3
    };
    constexpr uint16_t BASE_CAN_ID = 0x120;
    constexpr uint16_t BASE_CAN_ID_ENC = 0x1A0;
    struct Can{
        uint8_t can_id;
    public:
        int16_t target[2] = {0, 0};
        uint8_t mode[2] = {DUTY, DUTY};
        
    public:
        inline CanMessage encodeControl(){
            CanMessage msg(can_id, 8, true);
            for(uint8_t port = 0; port < 2; port++){
                // 拡張IDで送信内容の設定
                // 制御モードバリデーション
                if (mode[port] < DUTY || mode[port] > POSITION){
                    mode[port] = DUTY;
                    target[port] = 0;
                }
                msg.id += mode[port] << (6*port);

                // ターゲット値の設定
                CanCovert::int32_2_array(target[port], &msg.data[port*4]);
            }
            return msg;
        }

        
        inline CanMessage encodeParam(uint8_t config, int32_t port0_val, int32_t port1_val){
            CanMessage msg(can_id, 8, true);
            for(uint8_t port = 0; port < 2; port++){
                // 拡張IDで送信内容の設定
                // 制御モードバリデーション
                if (mode[port] < DUTY || mode[port] > POSITION){
                    msg.id += config << (6*port);
                } else {
                    msg.id &= ~(0b111111 << (6*port));
                }
            }
            // パラメータの設定   
            CanCovert::int32_2_array(port0_val, &msg.data[0]);
            CanCovert::int32_2_array(port1_val, &msg.data[4]);
            return msg;
        }

        inline void decodeControl(CanMessage& msg){
            if (msg.dlc < 8) return;
            if (!msg.isExtendedId) return;
            for(uint8_t port = 0; port < 4; port++){
                // モードの取得
                uint8_t mode_tmp = (msg.id >> (6 * port)) & 0b111111;
                if (mode[port] < DUTY || mode[port] > POSITION) continue;
                
                // 制御情報の取得・設定
                mode[port] = mode_tmp;
                target[port] = CanCovert::array_2_int32(&msg.data[port*4]);
            }
            return;
        }

        inline void decodeParam(CanMessage& msg, uint8_t &param0, int32_t &port0_val, uint8_t &param1, int32_t &port1_val){
            // バリデーションアウト用初期化
            param0 = 0;
            param1 = 0;
            if (msg.dlc < 8) return;
            if (!msg.isExtendedId) return;

            // パラメータインデクス取得
            param0 = msg.id & 0b111111;
            param1 = (msg.id >> 6) & 0b111111;

            // パラメータ設定値
            port0_val = CanCovert::array_2_int32(&msg.data[0]);
            port1_val = CanCovert::array_2_int32(&msg.data[4]);
            return;
        }
        
    public:
        Can(uint8_t child)
        : can_id(BASE_CAN_ID + (child << 18)){
            for (uint8_t port = 0; port < 4; port++)
            {
                target[port] = 0;
                mode[port] = DUTY;
            }
        }

        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            uint16_t can_id = msg.id >> 18;
            if(msg.isExtendedId){
                if (msg.id < BASE_CAN_ID) return -1;
                if (msg.id > BASE_CAN_ID + 0x0F) return -1;
                return msg.id - BASE_CAN_ID;    
            } else {
                if (msg.id < BASE_CAN_ID_ENC) return -1;
                if (msg.id > BASE_CAN_ID_ENC + 0x0F) return -1;
                return msg.id - BASE_CAN_ID_ENC;        
            }
            return -1;
            
        }
    };
}


namespace EncFeedBack{
    constexpr uint16_t BASE_CAN_ID = 0x1A0;
    struct Can{
        uint8_t child_id;
    public:
        int32_t current_enc[2];
        
    public:
        inline CanMessage encode(){
            CanMessage msg(BASE_CAN_ID + child_id, 8);
            for(uint8_t port = 0; port < 2; port++){
                CanCovert::int32_2_array(current_enc[port], &msg.data[port*4]);
            }
            return msg;
        }

        inline void decode(CanMessage& msg){
            if (msg.dlc < 8) return;
            for(uint8_t port = 0; port < 2; port++){
                current_enc[port] = CanCovert::array_2_int32(&msg.data[port*4]);
            }
            return;
        }
        
    public:
        Can(uint8_t child)
        : child_id(child){
            for (uint8_t port = 0; port < 2; port++)
            {
                current_enc[port] = 0;
            }
        }

        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id >= BASE_CAN_ID && msg.id <= BASE_CAN_ID + 0xF)return msg.id - BASE_CAN_ID;
            return -1;
        }
    };
}

namespace DutyFeedBack{
    constexpr uint16_t BASE_CAN_ID = 0x1B0;
    struct Can{
        uint8_t child_id;
    public:
        int16_t current_output[2];
        
    public:
        inline CanMessage encode(){
            CanMessage msg(BASE_CAN_ID + child_id, 4);
            for(uint8_t port = 0; port < 2; port++){
                CanCovert::int16_2_array(current_output[port], &msg.data[port*2]);
            }
            return msg;
        }

        inline void decode(CanMessage& msg){
            if (msg.dlc < 8) return;
            for(uint8_t port = 0; port < 2; port++){
                current_output[port] = CanCovert::array_2_int16(&msg.data[port*2]);
            }
            return;
        }
        
    public:
        Can(uint8_t child)
        : child_id(child){
            for (uint8_t port = 0; port < 2; port++)
            {
                current_output[port] = 0;
            }
        }

        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id >= BASE_CAN_ID && msg.id <= BASE_CAN_ID + 0xF)return msg.id - BASE_CAN_ID;
            return -1;
        }
    };
}