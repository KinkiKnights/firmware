#include <stm32f4xx_hal.h>
#include "board.hpp"
#include "../../include/control/ping_control.hpp"
#include "../../include/protocol/servo_msg.hpp"
#include "../../include/protocol/emergency_msg.hpp"
#include "../include/manage/all_manager.hpp"
#include "servo_control.hpp"
asm(".global _printf_float");
const uint16_t CONTROL_TERM_MS = 10;

int main()
{
    Board board(ServoMessage::SERVO_BASE_ID);
    PingControl ping(CONTROL_TERM_MS, board.can_id, &GlobalInterface::can1);
    PwmServoTest servo_test(board.buttons);
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    const bool DEBUG_MODE = (board.buttons[0]->getState() && board.buttons[1]->getState());
    printf("CAN HAT V2.0 :: CAN ID = %d\n", board.can_id);
    
    int32_t margin_ms;
    BoardManager::BoardManager manager;
    PingMessage ping_decoder;
    CanMessage rcv_msg;
    while (1)
    {
        // 実行更新
        board.leds[0]->flash(20);
        margin_ms = board.waitInterval(CONTROL_TERM_MS);

        // CAN受信メッセージ処理
        while (GlobalInterface::can_buff.get(rcv_msg)){
            board.leds[1]->flash(20);
            printf("\n RCV CAN  ");
            if (LivingMessage::LIVE_ID == rcv_msg.id){
                printf("is  LIVE ");
                manager.rcvLiveMsg(rcv_msg);
            } else {
                manager.rcvCanMsg(rcv_msg);
            }
        }
        
        // 各種更新
        manager.update(10);
        // printf("margin: %dms\n", margin_ms);

    }
    return 0;
}
