#include <stm32f4xx_hal.h>
#include "board.hpp"
Board* global_board;
#include "../../include/util/param.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "../../include/control/live_control.hpp"
#include "manage/all_manager.hpp"
asm(".global _printf_float");
const uint16_t CONTROL_TERM_MS = 10;

int main()
{
    Board board(Hat::Param::CAN_BASE_ID);
    global_board = &board;
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    printf("CAN HAT V2.0 :: CAN ID = %d\n", board.can_id);
    
    int16_t margin_ms;
    BoardManager::BoardManager manager;
    CanMessage rcv_msg;
    while (1)
    {
        // 実行更新
        board.leds[0]->flash(20);
        margin_ms = board.waitInterval(CONTROL_TERM_MS);

        // CAN受信メッセージ処理
        while (GlobalInterface::can_buff.get(rcv_msg)){
            board.leds[1]->flash(20);
            if (LivingMessage::LIVE_ID == rcv_msg.id){
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
