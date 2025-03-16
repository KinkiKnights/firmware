#pragma once
#define UART6
#define F4_CPU

#include "../../include/dev/F4/clock.h"
#include "../../include/dev/io.hpp"
#include "../../include/dev/F4/interface_md.hpp"
#include "./motor_control.hpp"

#ifndef CAN_CHILD_ID
#define CAN_CHILD_ID 15
#endif

/*==========================================================
 * ============ 基板定義
  ==========================================================*/
class Board{
private:
    // マイコンの各機能へのクロック供給を有効化します。
    // KKの基板では省エネ化の必要はないので、使わないクロックを無効化する必要はありません
    void clockEnable(){
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_RCC_TIM1_CLK_ENABLE();
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_TIM8_CLK_ENABLE();
        __HAL_RCC_TIM12_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_CAN2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }

public:
    uint16_t can_id;
    Button* buttons[2];
    Led* leds[2];
    Timer* tims[4];
    MotorPort* motors[4];
    EncoderTimer* encoder[2];

    // ボード初期化用コンストラクタ
    Board(uint16_t can_id_base){
        // CAN ID定義
        can_id = can_id_base + CAN_CHILD_ID;
        // システム初期化
        HAL_Init();
        SystemClockConfig();
        clockEnable();
        // 各種機能初期化
        
        leds[0] = new Led(GPIOC, GPIO_PIN_13);
        leds[1] = new Led(GPIOC, GPIO_PIN_14);
        tims[0] = new Timer(TIM2, 0);
        tims[1] = new Timer(TIM3, 0);
        tims[2] = new Timer(TIM8, 0);
        tims[3] = new Timer(TIM12, 0);


//         motors[0] = new MotorPort(
//             new Pwm(tims[1], TIM_CHANNEL_3, GPIOB, GPIO_PIN_0),
//             new Pwm(tims[2], TIM_CHANNEL_3, GPIOC, GPIO_PIN_8)
//         );
//         motors[1] = new MotorPort(
//             new Pwm(tims[1], TIM_CHANNEL_4, GPIOB, GPIO_PIN_1),
//             new Pwm(tims[2], TIM_CHANNEL_4, GPIOC, GPIO_PIN_9)
//         );
//         motors[2] = new MotorPort(
//             new Pwm(tims[1], TIM_CHANNEL_2, GPIOA, GPIO_PIN_7),
//             new Pwm(tims[3], TIM_CHANNEL_3, GPIOB, GPIO_PIN_10)
//         );
//         motors[3] = new MotorPort(
//             new Pwm(tims[3], TIM_CHANNEL_2, GPIOB, GPIO_PIN_15),
//             new Pwm(tims[0], TIM_CHANNEL_4, GPIOB, GPIO_PIN_2)
//         );
// #ifdef UART6
        GlobalInterface::debug_port.init();
// #endif
//         GlobalInterface::can2.init(leds[1], leds[2]);

    }

public:
    int32_t waitInterval(uint32_t term_ms){
        static uint32_t last_interval_ms = 0;
        uint32_t limit = term_ms + last_interval_ms;
        uint32_t t = HAL_GetTick();
        // 余剰秒数の算出
        int32_t margin_ms = limit - t;
        while (t < limit)
        {
            t = HAL_GetTick();
        }        
        last_interval_ms = t;
        return margin_ms;
    }
};