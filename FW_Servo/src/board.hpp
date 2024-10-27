#pragma once
#define CAN1
#define UART1
#include <stm32f3xx_hal.h>
#include "../include/dev/clock.h"
#include "../include/dev/io.hpp"
#include "../include/dev/interface.hpp"

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
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }

public:
    uint16_t can_id;
    Button* buttons[0];
    Led* led[3];
    Timer* tims[2];
    Pwm* pwms[2];

    // ボード初期化用コンストラクタ
    Board(uint16_t can_id_base){
        // CAN ID定義
        can_id = can_id_base + CAN_CHILD_ID;
        // システム初期化
        HAL_Init();
        SystemClockConfig();
        clockEnable();
        // 各種機能初期化
        led[0] = new Led(GPIOA, GPIO_PIN_15);
        led[1] = new Led(GPIOB, GPIO_PIN_3);
        led[2] = new Led(GPIOB, GPIO_PIN_4);
        tims[0] = new Timer(TIM2, 31);
        tims[1] = new Timer(TIM3, 31);
        pwms[0] = new Pwm(tims[0], TIM_CHANNEL_1, GPIOA, GPIO_PIN_0);
        
    // インターフェイスの初期化
#ifdef UART1
    GlobalInterface::debug_port.init();
#endif
#ifdef CAN1
    GlobalInterface::can1.init(led[1], led[2]);
#endif

    }
};