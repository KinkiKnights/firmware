#pragma once
#include <stdint.h>
#include "./can_msg.hpp"

namespace EPB{
    constexpr uint16_t CAN_ID = 0x000;
    struct Can{
    public:
        bool is_safety = false; // 安全状態

    public:

        CanMessage encode(){
            CanMessage msg;
            msg.id = CAN_ID;
            msg.dlc = 1;
            msg.data[0] = (is_safety) ? 1 : 0;
            return msg;
        }

        bool decode(CanMessage& msg){
            is_safety = (msg.data[0] == 1);
            return is_safety;
        }
        
    public:
        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id == CAN_ID) return 0;
            return -1;
        }
    };
};

namespace EPBFb{
    constexpr uint16_t CAN_ID = 0x001;
    struct Can{
    public:
        bool is_safety = false; // 安全状態(総合)
        bool is_safety_epb = false; // 物理ボタン状態

    public:
        CanMessage encode(){
            CanMessage msg;
            msg.id = CAN_ID;
            msg.dlc = 2;
            msg.data[0] = (is_safety) ? 1 : 0;
            msg.data[1] = (is_safety_epb) ? 1 : 0;
            return msg;
        }

        bool decode(CanMessage& msg){
            is_safety = (msg.data[0] == 1);
            is_safety_epb = (msg.data[1] == 1);
            return is_safety;
        }
        
    public:
        /**
         * @brief CAＮメッセージの子IDを返す。不適合であれば-1
         * @return 各基板側ファームからの利用が想定されている。
         */
        inline static int8_t getChildID(CanMessage& msg){
            if (msg.id == CAN_ID) return 0;
            return -1;
        }
    };
};