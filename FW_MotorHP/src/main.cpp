#define F4MD
#include <stm32f4xx_hal.h>
#include "board.hpp"
// #include "../../include/protocol/_protocol.hpp"
// #include "../../include/control/live_control.hpp"
// #include "./motor_control.hpp"
// #include "./encoder_control.hpp"
asm(".global _printf_float");
const uint16_t CONTROL_TERM_MS = 10;

// デバッグモード選択
#define NORMAL_RUN
#define DEBUG_STANDALONE
// #define ENCODER_MODE
// #define DEBUG_CAN_RECEIVER
// #define DEBUG_CAN_SENDER

int main()
{
    Board board(Motor::Param::CAN_BASE_ID);
    
    // while (1)
    // {
    //     // 実行点滅
    //     board.leds[0]->flash(20);
    //     board.leds[1]->flash(5);
    //     int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
    //     printf("Hello\n");
    // }
//     LiveControl ping(CONTROL_TERM_MS, board.can_id, &GlobalInterface::can1);
    MotorControl motor_control0(CONTROL_TERM_MS, board.motors);
    MotorControl motor_control1(CONTROL_TERM_MS, &(board.motors[2]));
    MotorTest motor_test(0.3f);
    
    /*================================
    ロジックの初期化
    ==================================*/
    HAL_Delay(100);
    printf("MotorDriverHipower V1.0 :: CAN ID = %d , %d\n", board.can_id, board.can_id + 1);
    
    // EPB::Can epb_decoder;


// #ifdef NORMAL_RUN// 通常モード実装
//     while (!DEBUG_MODE)
//     {
//         // 実行点滅
//         board.leds[0]->flash(20);
//         int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
        
//         // CAN受信メッセージ処理
//         CanMessage rcv_msg;
//         while (GlobalInterface::can_buff.get(rcv_msg)){
//             board.leds[1]->flash(6);
//             if (Motor::Can::isMe(rcv_msg, board.can_id)){
//                 board.leds[2]->flash(6);
//                 motor_control0.setControl(rcv_msg);
                
//                 printf("GetCAN              \n");
//             }
            
//             if (Motor::Can::isMe(rcv_msg, board.can_id + 1)){
//                 board.leds[2]->flash(6);
//                 motor_control1.setControl(rcv_msg);
                
//                 printf("GetCAN              \n");
//             }
//         }
//         printf("margin: %dms\n", margin_ms);
//         ping.update(CONTROL_TERM_MS - margin_ms);
//         motor_control0.update();
//         motor_control1.update();
//     }
// #endif

// #ifdef DEBUG_STANDALONE // 自己完結型デバッグモード(通常)
    printf("Start Standalone Debug\n");
    while (1)
    {           
        CanMessage p1;
        motor_test.update(board.can_id, p1);
        motor_control0.setControl(p1);
        motor_control1.setControl(p1);
        motor_control0.update();
        motor_control1.update();
        board.leds[0]->flash(2);
        board.leds[1]->flash(2);
        int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
        // printf("port0Counter: %d => %d ; margin: %dms\n", servo_control.getCounter(0), servo_control.getCounterCurrent(0), margin_ms);
    }
// #endif
// #ifdef DEBUG_CAN_RECEIVER // CAN受信型デバッグモード(動作)
//     uint16_t can_id = Motor::Param::CAN_BASE_ID;
//     printf("Start CAN Receiver Debug\n");

//     while (1)
//     {
//         // 実行点滅
//         board.leds[0]->flash(5);
//         int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
        
//         // CAN受信メッセージ処理
//         CanMessage rcv_msg;
//         while (GlobalInterface::can_buff.get(rcv_msg)){
//             board.leds[1]->flash(6);
//             if (Motor::Can::isMe(rcv_msg, can_id)){
//                 board.leds[2]->flash(6);
//                 motor_control.setControl(rcv_msg);
//             } else{
//                 printf("                    ElseID: %d\n", rcv_msg.id);
//             }
//         }

//         motor_control.update();
//     }
// #endif
// #ifdef DEBUG_CAN_SENDER // CAN送信型デバッグモード(非動作)
//     printf("Start CAN Sender Debug\n");
//     while (1)
//     {           
//         // デバッグモード
//         CanMessage p1;
//         motor_test.update(0, p1);
//         GlobalInterface::can1.send(p1);
//         board.leds[1]->flash(2);
//         board.leds[2]->flash(2);
//         int16_t margin_ms = board.waitInterval(CONTROL_TERM_MS);
//         printf("margin: %dms\n", margin_ms);
//     }
// #endif
    return 0;
}
