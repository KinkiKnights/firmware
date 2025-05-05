#include <stm32f3xx_hal.h>
#include "board.hpp"
#include "./servo_control.hpp"
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
    PwmServoModel servo_control(board.pwms);
    PwmServoTest servo_test(board.buttons);
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    const bool DEBUG_MODE = (board.buttons[0]->getState() && board.buttons[1]->getState());
    printf("PWM Servo V2.0 :: CAN ID = %d\n", board.can_id);
    
#ifdef NORMAL_RUN// 通常モード実装    
    while (!DEBUG_MODE)
    {
        // 実行点滅
        board.leds[0]->flash(20);
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);

        // CAN受信メッセージ処理
        CanMessage rcv_msg;
        while (GlobalInterface::can_buff.get(rcv_msg)){
            board.leds[1]->flash(6);
            if (ServoPwm::Can::isBoardCanID(rcv_msg, board.can_id)){
                board.leds[2]->flash(6);
                servo_control.setControl(rcv_msg);
            }
        }

        // 各種更新処理
        ping.update(CONTROL_TERM_MS - margin_ms, board.power->getState());
        servo_control.update(CONTROL_TERM_MS);
        
        printf("margin: %dms\n", margin_ms);
    }
#endif

#ifdef DEBUG_STANDALONE // 自己完結型デバッグモード(通常)
    printf("Start Standalone Debug\n");
    while (1)
    {           
        CanMessage p1,p2;
        servo_test.genMsg(board.can_id, p1, p2);
        servo_control.setControl(p1);
        servo_control.setControl(p2);
        servo_control.update(CONTROL_TERM_MS);
        board.leds[1]->flash(2);
        board.leds[2]->flash(2);
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
    }
#endif
#ifdef DEBUG_CAN_RECEIVER // CAN受信型デバッグモード(動作)
    uint16_t can_id = ServoPwm::Param::CAN_BASE_ID;
    
    printf("Start CAN Receiver Debug\n");
    while (1)
    {
        // 実行点滅
        board.leds[0]->flash(5);
        CanMessage rcv_msg;
        while (GlobalInterface::can_buff.get(rcv_msg)){
            board.leds[1]->flash(6);
            if (ServoPwm::Can::isBoardCanID(rcv_msg, can_id)){
                board.leds[2]->flash(6);
                servo_control.setControl(rcv_msg);
            } else{
                printf("                    ElseID: %d\n", rcv_msg.id);
            }
        }
        servo_control.update(CONTROL_TERM_MS);
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
        // printf("port0Counter: %d => %d ; margin: %dms\n", servo_control.getCounter(0), servo_control.getCounterCurrent(0), margin_ms);
    }
#endif
#ifdef DEBUG_CAN_SENDER // CAN送信型デバッグモード(非動作)
    printf("Start CAN Sender Debug\n");
    while (1)
    {           
        CanMessage p1,p2;
        servo_test.genMsg(0, p1, p2);
        GlobalInterface::can1.send(p1);
        GlobalInterface::can1.send(p2);
        board.leds[1]->flash(2);
        board.leds[2]->flash(2);
        printf("Counter: %d, ", servo_test.getCounter());
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
        printf("margin: %dms\n", margin_ms);
    }
#endif
    return 0;
}
