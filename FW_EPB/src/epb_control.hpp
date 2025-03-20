#pragma once
#include <math.h>
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/io.hpp"

/**
 * @brief PWMサーボのモデル制御を実施する
 */

class EpbModel{
private:
    bool is_epb_soft = true;
    bool is_epb_browser = true;
    bool is_epb_phisy = false;
    bool is_epb_all = false;
    bool is_killed = false;

    void setLed(bool flag, GpioOut* led){
        if (flag)
            led->on();
        else
            led->off();
    }

    bool getNum(char* MSB, uint8_t& num){
        bool success = true;
        num  = 0;
        if (MSB[0] > 0x2f && MSB[0] < 0x3a) num = MSB[0] - 0x30;
        else if (MSB[0] > 0x40 && MSB[0] < 0x47) num = MSB[0] - 0x41 + 0x0A;
        else success = false;
        
        num = num << 4;

        if (MSB[1] > 0x2f && MSB[1] < 0x3a) num += MSB[1] - 0x30;
        else if (MSB[1] > 0x40 && MSB[1] < 0x47) num += MSB[1] - 0x41 + 0x0A;
        else success = false;

        return success;
    }

    uint8_t cmd[7];
    bool is_rcv = false;
    char tmp_buff[50];
    uint8_t buff_idx = 0;

    void updateTrace(Board* board){

        // 無線通信モジュール処理
        while (!GlobalInterface::dma_port.RX_IsEmpty()){
            char tmp_c = GlobalInterface::dma_port.RX_Read();
            // 行末文字
            if (tmp_c == '\r'){
                tmp_buff[buff_idx] = 0;
                printf("\nRCV TXT(%d) ==> ", buff_idx);
                printf(tmp_buff);

                if (buff_idx!=34){
                    printf("Wrong parce result......\n");
                    continue;
                }

                bool success = true;
                for (uint8_t cidx = 0; cidx < 7; cidx++){
                    success = getNum(&tmp_buff[14 + cidx * 3], cmd[cidx]);
                    if (!success) break;
                }

                if (success){
                    is_rcv = true;
                    printf("\nRCV_CMD ==>%d, %d, %d, %d, %d, %d, %d", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6]);
                    // この信号のみ0時true
                    if (cmd[0] & 0b1){
                        is_epb_browser = false;
                    } else {
                        is_epb_browser = true;
                    }
                    board->leds_status[0]->toggle();
                } else {
                    printf("Wrong exchange...\n");
                }
                
                continue;
            }
            // 行頭文字
            else if (tmp_c == '\n'){
                buff_idx = 0;
                continue;
            }
            else {
                tmp_buff[buff_idx++] = tmp_c;
            }
        }
    }

    
    uint8_t hp = 0;
    bool is_rcv_hp = false;
    char hp_buff[50];
    uint8_t hp_buff_idx = 0;
    void updateClient(Board* board){
        // クライアントモジュール
        while (!GlobalInterface::debug_port.RX_IsEmpty()){
            char tmp_c = GlobalInterface::debug_port.RX_Read();
            // 行末文字
            if (tmp_c == '\r'){
                hp_buff[hp_buff_idx] = 0;
                printf("\nRCV TXT(%d) ==> ", hp_buff_idx);
                printf(hp_buff);

                if (hp_buff_idx!=23){
                    printf("Wrong parce result......\n");
                    continue;
                }


                bool success = getNum(&hp_buff[9], hp);

                if (success){
                    is_rcv_hp = true;
                    printf("Get HP %d\n", hp); 
                    board->leds_status[1]->toggle();
                        
                } else {
                    printf("Wrong exchange...\n");
                }
                
                continue;
            }
            // 行頭文字
            else if (tmp_c == '\n'){
                hp_buff_idx = 0;
                continue;
            }
            else {
                hp_buff[hp_buff_idx++] = tmp_c;
            }
        }
    }

public:
    void update(Board* board){
        // 状態の取得
        board->leds[0]->flash(2);
        is_killed =  !board->is_killed->getState();
        is_epb_phisy = !board->is_epb_phisic->getState();
        is_epb_all = !board->is_epb_relay->getState();
        if (board->buttons[0]->isPush()){
            is_epb_soft = !is_epb_soft;
        }

        // 通信の取得
        updateTrace(board);
        updateClient(board);

        // 出力の変更
        setLed(is_epb_soft && !is_killed && is_epb_browser, board->epb_soft);
        

        if (is_killed)
            board->leds_status[2]->flash(2);
        else
            board->leds_status[2]->off();

        if (is_epb_soft)
            board->leds[1]->on();
        else
            board->leds[1]->flash(20);

        if (is_epb_all)
            board->leds[2]->on();
        else
            board->leds[2]->flash(20);

        // 通信の発信
        CanMessage msg;
        msg.id = 0x01;
        msg.dlc = 8;
        msg.data[0] = ((is_epb_soft && !is_killed) << 7) + hp;
        for (uint8_t idx = 0; idx < 7; idx++){
            msg.data[idx + 1] = cmd[idx];
        }
        GlobalInterface::can1.send(msg);

    }
};