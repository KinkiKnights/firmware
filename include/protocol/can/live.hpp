#pragma once
#include <stdint.h>
#include "can_msg.hpp"

namespace Live{
    constexpr uint16_t CAN_ID = 0x002;
    struct Can{
    public:
        uint16_t board_id = 0;  // 基板ID
        uint16_t serial_id = 0; // 基板固有ID
        uint16_t run_time = 0;  // 基板動作時間
        uint8_t trans_time = 0; // 1周期処理時間

    public:
        inline CanMessage encode(){
            CanMessage msg;
            msg.id = CAN_ID;
            msg.dlc = 7;
            CanCovert::uint16_2_array(board_id, &msg.data[0]);
            CanCovert::uint16_2_array(serial_id, &msg.data[2]);
            CanCovert::uint16_2_array(run_time, &msg.data[4]);
            msg.data[6] = trans_time;
            return msg;
        }

        inline void decode(CanMessage& msg){
            if (msg.dlc < 7) return;
            board_id = CanCovert::array_2_uint16(&msg.data[0]);
            serial_id = CanCovert::array_2_uint16(&msg.data[2]);
            run_time = CanCovert::array_2_uint16(&msg.data[4]);
            trans_time = msg.data[6];
            return;
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
}