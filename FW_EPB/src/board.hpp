#pragma once
#define CAN1
#define UART1DMA
#define UART2
#include <stm32f3xx_hal.h>
#include "../../include/dev/F3/clock.h"
#include "../../include/dev/F3/interface.hpp"
#include "../../include/dev/io.hpp"

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
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();
    }

public:
    uint16_t can_id;
    Button* buttons[2];
    Led* leds[3];
    Led* leds_status[3];
    GpioIN* is_epb_phisic;
    GpioIN* is_epb_relay;
    GpioIN* is_killed;
    GpioOut* epb_soft;

    // ボード初期化用コンストラクタ
    Board(uint16_t can_id_base){
        // CAN ID定義
        can_id = 0;
        // システム初期化
        HAL_Init();
        SystemClockConfig();
        clockEnable();
        // 通常処理用ディスプレイ
        buttons[0] = new Button(GPIOB, GPIO_PIN_7);
        buttons[1] = new Button(GPIOB, GPIO_PIN_5);
        leds[0] = new Led(GPIOB, GPIO_PIN_3);
        leds[1] = new Led(GPIOB, GPIO_PIN_4);
        leds[2] = new Led(GPIOB, GPIO_PIN_6);
        // 状態表示
        leds_status[0] = new Led(GPIOA, GPIO_PIN_5);
        leds_status[1] = new Led(GPIOA, GPIO_PIN_6);
        leds_status[2] = new Led(GPIOA, GPIO_PIN_7);
        // 非常停止フィードバック・非常停止出力
        is_killed = new GpioIN(GPIOA, GPIO_PIN_4);
        is_epb_phisic = new GpioIN(GPIOA, GPIO_PIN_0);
        is_epb_relay = new GpioIN(GPIOA, GPIO_PIN_1);
        epb_soft = new GpioOut(GPIOA, GPIO_PIN_2, 1);
        
        

        leds[0]->on();
        leds[1]->on();
        // インターフェイスの初期化
#ifdef UART1
        GlobalInterface::debug_port.init();
#endif
#ifdef UART1DMA
        GlobalInterface::debug_port.init();
#endif
#ifdef UART2
        GlobalInterface::dma_port.init();
#endif
#ifdef CAN1
        GlobalInterface::can1.init(leds[1], leds[2]);
#endif

        leds[2]->on();
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