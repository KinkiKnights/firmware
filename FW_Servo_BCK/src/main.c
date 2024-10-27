#define bool uint8_t
#define true 1
#define false 0

#include <stm32f3xx_hal.h>
#include "board/board.h"
#include "board/clock.h"
#include "board/init.h"
// 処理機能
#include "control/epb.h"
#include "control/live.h"
#include "logic/id.h"
#include "logic/test.h"
#include "logic/normal.h"
#include "logic/adjust.h"

int main()
{
    HAL_Init();
    SystemClockConfig();
    boardConfig();
    initServo();
    // 子IDの取り出し
    board.can_id = CANID_PWM_BASE_ID + CAN_CHILD_ID;
    /*================================
    ロジックの初期化
    ==================================*/

    HAL_Delay(100);
    printf("PWM Servo V1.0 :: CAN ID = %d -> %d + %d\n", board.can_id, CANID_PWM_BASE_ID , CAN_CHILD_ID);
    if (buttonState(board.buttons[0]) && buttonState(board.buttons[1]))
        idChangeRun();
    else if (buttonState(board.buttons[0]))
        adjustRun(); 
    else if (buttonState(board.buttons[1]))
        pwmTestRun();        
    else
        servoMainRun();

    return 0;
}
