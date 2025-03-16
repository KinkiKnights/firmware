#pragma once
#define CAN1
#define UART1
#include <stm32f3xx_hal.h>
#include "../../include/dev/F3/clock.h"
#include "../../include/dev/F3/interface.hpp"
#include "../../include/dev/io.hpp"
#include "./bldc_control.hpp"

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
    Button* buttons[2];
    Led* leds[3];
    Timer* tims[2];
    BLDCMotor* motors[2];

    // ボード初期化用コンストラクタ
    Board(uint16_t can_id_base){
        // CAN ID定義
        can_id = can_id_base + CAN_CHILD_ID;
        // システム初期化
        HAL_Init();
        SystemClockConfig();
        clockEnable();
        // 各種機能初期化
        
        leds[0] = new Led(GPIOB, GPIO_PIN_3);
        leds[1] = new Led(GPIOB, GPIO_PIN_4);
        leds[2] = new Led(GPIOB, GPIO_PIN_6);
        buttons[0] = new Button(GPIOB, GPIO_PIN_5);
        buttons[1] = new Button(GPIOB, GPIO_PIN_7);
        tims[0] = new Timer(TIM2, 0, 800);
        tims[1] = new Timer(TIM3, 0, 800);
        // Motor1
        motors[0] = new BLDCMotor(
            new Pwm(tims[0], TIM_CHANNEL_3, GPIOA, GPIO_PIN_2)
        );
        motors[1] = new BLDCMotor(
            new Pwm(tims[1], TIM_CHANNEL_1, GPIOA, GPIO_PIN_6)
        );
        // インターフェイスの初期化
#ifdef UART1
        GlobalInterface::debug_port.init();
#endif
#ifdef CAN1
        GlobalInterface::can1.init(leds[1], leds[2]);
#endif

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