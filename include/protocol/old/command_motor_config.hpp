#pragma once
#include <stdint.h>
#include "byte_conversion.hpp"

namespace Command
{
    struct MotorConfig{
        static const uint8_t MOTOR_CNFG_CMD = 13;
        uint8_t child_id = 0;
        uint8_t port = 0;
        uint8_t rec = 0;
        bool rev = false;
        float pos_p,pos_i,pos_d,spd_p,spd_i,spd_d;
        int32_t pos_lim_p = 0;
        int32_t pos_lim_m = 0;
        int32_t spd_lim = 0;

        uint8_t encode(uint8_t* frame){
            uint8_t dlc = 40; 
            frame[0] = MOTOR_CNFG_CMD;
            frame[1] = dlc;
            frame[2] = child_id;
            frame[3] = rec;
            float_to_array(frame + 4, pos_p);
            float_to_array(frame + 8, pos_i);
            float_to_array(frame + 12, pos_d);
            float_to_array(frame + 16, spd_p);
            float_to_array(frame + 20, spd_i);
            float_to_array(frame + 24, spd_d);
            int32_to_array(frame + 28, pos_lim_p);
            int32_to_array(frame + 32, pos_lim_m);
            int32_to_array(frame + 36, spd_lim);
            return dlc + 2;
        }
        void decode(uint8_t* frame){
            child_id = frame[2];
            rec = frame[3];
            array_to_float(frame + 4 , pos_p);
            array_to_float(frame + 8 , pos_i);
            array_to_float(frame + 12, pos_d);
            array_to_float(frame + 16, spd_p);
            array_to_float(frame + 20, spd_i);
            array_to_float(frame + 24, spd_d);
            array_to_int32(frame + 28, pos_lim_p);
            array_to_int32(frame + 32, pos_lim_m);
            array_to_int32(frame + 36, spd_lim);
        }
    };
    struct MotorConfigFeedback{
        static const uint8_t MOTOR_CNFG_FEEDBACK = 14;
        static const uint8_t MOTOR_NUM = 4;
        uint8_t child_id = 0;
        uint8_t port_num = 0;
        uint8_t rec[MOTOR_NUM];
        uint8_t encode(uint8_t* frame){
            uint8_t dlc = port_num + 1; 
            frame[0] = MOTOR_CNFG_FEEDBACK;
            frame[1] = dlc;
            frame[2] = child_id;
            for (uint8_t idx = 0; idx < port_num; idx++){
                frame[idx + 3] = rec[idx];
            }
            return dlc + 2;
        }
        void decode(uint8_t* frame, uint8_t dlc){
            port_num = frame[1] -1; // dlc抽出
            child_id = frame[2];
            for (uint8_t idx = 0; idx < port_num; idx++){
                rec[idx] = frame[idx + 3];
            }
        }
    };
    
} // namespace Command
