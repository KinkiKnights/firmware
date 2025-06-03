#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"

#include "./life_cycle.hpp"
#include "./manage_ics.hpp"
#include "./manage_pwm.hpp"
#include "./manage_gm6020.hpp"

namespace BoardManager
{
    // 生存確認によって各基板有効化
    class BoardManager{
    private:
        PwmManager pwm;
        IcsManager ics;
        Gm6020Manager gm6020;

        Live::Can live_decoder;
        Live::Serial live_encoder;

    public:
        // 各基板Update
        BoardManager(){}
        
        void update(uint16_t update_ms){
            // シリアルの受信
            updateCommandRcv();
            // 各モジュールの更新
            pwm.update(update_ms);
            ics.update(update_ms);
            gm6020.update(update_ms);
        }
        void rcvCanMsg(CanMessage& msg){
            if(pwm.rcvCan(msg)) return;
            if(ics.rcvCan(msg)) return;
            if(gm6020.rcvCan(msg)) return;

            // LIVEメッセージなら統一Manager側で処理
            if (Live::Can::getChildID(msg)){
                live_decoder.decode(msg);
                // 各マネージャの更新処理
                do {
                    if (pwm.rcvLive(live_decoder)) break;
                    if (ics.rcvLive(live_decoder)) break;
                    // if (gm6020.rcvLive(live_decoder)) break;　GM6020にはlIVE機能は無し
                    // 該当する基板が無ければラズパイには情報送信しない
                    return;
                }while (0);

                // マネージャの記録
                // ToDo Liveのシリアル実装は後日
            }
        }
    public:
        bool rcvCommand(uint8_t* frames){
            if(pwm.rcvCommand(frames)) return true;
            if(ics.rcvCommand(frames)) return true;
            if(gm6020.rcvCommand(frames)) return true;
            printf("NONE Match(%d)\n", frames[0]);
            return false;
        }
    private:
        static const uint8_t TMP_NUM = 100;
        static constexpr uint8_t DLC_IDX = 1;
        uint8_t blank_counter = 0;
        uint8_t tmp_index = 0;
        uint32_t check_sum = 0;
        uint8_t cmd_temporary[TMP_NUM];
        void updateCommandRcv(){
            // printf("start Serial");
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
                            // printf("\nChecksum calc is failed(%ld(calc) != %d(frame))...", check_sum, tmp_c);
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
