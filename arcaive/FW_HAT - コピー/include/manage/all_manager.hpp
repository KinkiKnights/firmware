#pragma once
#define F4_CPU

#include "../../include/protocol/_protocol.hpp"
#include "../../include/protocol/live_msg.hpp"
#include "../../include/dev/F4/interface.hpp"
#include "../command/_command.hpp"
#include "./_manage.hpp"

namespace BoardManager
{
    // 生存確認によって各基板有効化
    class BoardManager{
    private:
        EpbManager* epb[EpbManager::MANAGER_NUM];
        PwmManager* pwm[PwmManager::MANAGER_NUM];
        MotorManager* motor[MotorManager::MANAGER_NUM];
        template <class T>
        void genManager(T** array){
            uint8_t num = T::MANAGER_NUM;
            for (uint8_t i = 0; i < num; i++) {
                array[i] = new T(i);
            }
        }
        template <class T>
        void updateManager(T** array, uint16_t update_ms){
            uint8_t num = T::MANAGER_NUM;
            for (uint8_t i = 0; i < num; i++) {
                array[i]->update(update_ms);
            }
        }
        template <class T>
        bool rcvLiveMsg(T** array, LivingMessage& msg){
            int8_t child_id = T::isMeID(msg.can_id);
            if (child_id < 0) return false;
            if (child_id >= T::MANAGER_NUM) return false;
            array[child_id]->rcvLiving(msg);
            return true;
        }
        template <class T>
        bool rcvCanMsg(T** array, CanMessage& msg){
            int8_t child_id = T::isMeID(msg.id);
            if (child_id < 0) return false;
            if (child_id >= T::MANAGER_NUM) return false;
            array[child_id]->rcvCan(msg);
            return true;
        }
        template <class T>
        bool rcvCmdMsg(T** array, uint8_t* frames){
            int8_t child_id = T::isMeCmd(frames);
            if (child_id < 0) return false;
            if (child_id >= T::MANAGER_NUM) return false;
            array[child_id]->rcvCommand(frames);
            return true;
        }
    private:
        LivingMessage live_encoder;
        Command::Emergency epb_encoder;
        Command::EmergencyFeedback epb_fb_encoder;
        Command::EncoderFeedback enc_encoder;
        Command::PwmServo pwm_encoder;
        Command::MotorControl mtrctrl_encoder;
        Command::MotorConfig mtrcfg_encoder;
        Command::MotorConfigFeedback mtrcfg_fb_encoder;
    public:
        // 各基板Update
        BoardManager(){
            genManager<EpbManager>(epb);
            genManager<PwmManager>(pwm);
            genManager<MotorManager>(motor);
        }
        void update(uint16_t update_ms){
            updateCommandRcv();
            updateManager<EpbManager>(epb, update_ms);
            updateManager<PwmManager>(pwm, update_ms);
            updateManager<MotorManager>(motor, update_ms);
        }
        void rcvLiveMsg(CanMessage& msg){
            // 共通デコード
            live_encoder.decode(msg);
            printf("Get Live(%d,%d)   =>", msg.data[0], msg.data[1]);
            if(rcvLiveMsg<EpbManager>(epb, live_encoder)) return;
            printf("EPB:False  ");
            if(rcvLiveMsg<PwmManager>(pwm, live_encoder)) return;
            printf("Pwm:False  ");
            if(rcvLiveMsg<MotorManager>(motor, live_encoder)) return;
            printf("Motor:False  ");
        }
        void rcvCanMsg(CanMessage& msg){
            if(rcvCanMsg<EpbManager>(epb, msg)) return;
            if(rcvCanMsg<PwmManager>(pwm, msg)) return;
            if(rcvCanMsg<MotorManager>(motor, msg)) return;
        }
        bool rcvCommand(uint8_t* frames){
            printf("RCV UART Command");
            if(rcvCmdMsg<EpbManager>(epb, frames)) return true;
            if(rcvCmdMsg<PwmManager>(pwm, frames)) return true;
            if(rcvCmdMsg<MotorManager>(motor, frames)) return true;
            printf("NONE Match(%d)\n", frames[0]);
            return false;
        }
        
        static const uint8_t TMP_NUM = 100;
        static constexpr uint8_t DLC_IDX = 1;
        uint8_t blank_counter = 0;
        uint8_t tmp_index = 0;
        uint32_t check_sum = 0;
        uint8_t cmd_temporary[TMP_NUM];
        void updateCommandRcv(){

            uint8_t tmp_c;
            while(!GlobalInterface::dma_uart.RX_IsEmpty()){
                tmp_c = GlobalInterface::dma_uart.RX_Read();
                printf("%d,", (uint8_t)tmp_c);

                // 先頭文字列探索
                if (blank_counter < 2){
                    if(tmp_c == 0xF0)
                        blank_counter++;
                    else
                        blank_counter = 0;
                    printf(": blank search:%d\n", blank_counter);
    
                }
                // コマンド先頭探索
                else if (tmp_index == 0){
                    // 初期文字列が続く場合はコマンド先頭探索
                    if (tmp_c == 0xF0) continue;
                    cmd_temporary[tmp_index++] = tmp_c;
                    check_sum = tmp_c;
                    printf(":cmd type\n");
                }
                // コマンド処理
                else {
                    cmd_temporary[tmp_index++] = tmp_c;
                    // コマンド受信中間処理
                    if (tmp_index < (cmd_temporary[DLC_IDX] + 3)){
                        check_sum += tmp_c;
                    }
                    // コマンド受信完了処理
                    else {
                        // チェックサム正常計算時
                        if (check_sum % 0x100 == tmp_c)
                            rcvCommand(cmd_temporary);
                        // チェックサム異常時処理
                        else 
                            printf("\nChecksum calc is failed(%d(calc) != %d(frame))...", check_sum, tmp_c);
                        // 初期化処理
                        blank_counter = 0;
                        tmp_index = 0;
                        printf("\n");
                    }
                }

            }
        }
    };
} // namespace BoardManager
