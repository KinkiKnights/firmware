#include <stm32f3xx_hal.h>
#include "board.hpp"
#include "./epb_control.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "../../include/control/live_control.hpp"
asm(".global _printf_float");
const uint16_t CONTROL_TERM_MS = 10;

// デバッグモード選択
#define NORMAL_RUN
#define DEBUG_STANDALONE
// #define DEBUG_CAN_RECEIVER
// #define DEBUG_CAN_SENDER

int main()
{
    Board board(ServoPwm::Param::CAN_BASE_ID);
    LiveControl ping(CONTROL_TERM_MS, board.can_id, &GlobalInterface::can1);
    EpbModel epb_control;
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    const bool DEBUG_MODE = (board.buttons[0]->getState() && board.buttons[1]->getState());
    printf("EPB CoreMaster V1.0 :: CAN ID = %d\n", board.can_id);
    
    while (!DEBUG_MODE)
    {
        // 実行点滅
        ping.update(10);
        epb_control.update(&board);
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);

    }
    return 0;
}
