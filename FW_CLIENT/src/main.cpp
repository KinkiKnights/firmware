#include <stm32f4xx_hal.h>
#include "board.hpp"
#include "../../include/util/param.hpp"
#include "../../include/protocol/_protocol.hpp"
#include "../../include/control/live_control.hpp"
#include "manage/all_manager.hpp"
asm(".global _printf_float");
const uint16_t CONTROL_TERM_MS = 10;

int main()
{
    Board board(Hat::Param::CAN_BASE_ID);
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
        // board.leds[0]->flash(20);
        margin_ms = board.waitInterval(CONTROL_TERM_MS);
        if (board.buttons[2]->getState())
            board.leds[1]->on();
        else
            board.leds[1]->off();
        if (board.buttons[3]->getState())
            board.leds[0]->on();
        else
            board.leds[0]->off();
       // 各種更新

        char tmp_c;
        while(!GlobalInterface::dma_uart.RX_IsEmpty()){
            tmp_c = GlobalInterface::dma_uart.RX_Read();
            
        }
       
        printf("margin: %dms\n", margin_ms);

        // リレー制御
        // 通信クライアント(パイソン)


    }
    return 0;
}
