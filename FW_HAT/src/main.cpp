#include <stm32f4xx_hal.h>
#include "board.hpp"
Board* global_board;
#include "../../include/util/param.hpp"
#include "../../include/protocol/can/can_msg.hpp"
#include "../../include/protocol/serial/serial_msg.hpp"
#include "../../include/control/live_control.hpp"
#include "manage/manage.hpp"
#include "test.hpp"
asm(".global _printf_float");
constexpr uint8_t CYCLE_MS = 5000;

int main()
{
    Board board(Hat::Param::CAN_BASE_ID);
    global_board = &board;
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    printf("CAN HAT V2.0 :: CAN ID = %d\n", board.can_id);
    
    BoardManager::BoardManager manager;
    CanMessage rcv_msg;

    // 単体デバッグ実行用
    
    while (1)
    {
        // 実行更新
        board.leds[0]->flash(20);
        int16_t margin_ms = board.waitInterval(50);

        // CAN受信メッセージ処理
        while (GlobalInterface::can_buff.get(rcv_msg)){
            board.leds[1]->flash(20);
            manager.rcvCanMsg(rcv_msg);
        }
        // 各種更新
        manager.update(CYCLE_MS);

        // 各種テスト
        Test::update(board, manager, CYCLE_MS);
        printf("margin: %dms\n", margin_ms);
    }
    return 0;
}
