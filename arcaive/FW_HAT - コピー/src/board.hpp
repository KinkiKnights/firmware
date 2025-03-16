#pragma once
#define F4_CPU
#define UART6
#define UART1DMA
#include <stm32f4xx_hal.h>
#include "../../include/dev/F4/clock.h"
#include "../../include/dev/io.hpp"
#include "../../include/dev/F4/interface.hpp"

/*==========================================================
 * ============ 基板定義
  ==========================================================*/
class Board{
private:
    // マイコンの各機能へのクロック供給を有効化します。
    // KKの基板では省エネ化の必要はないので、使わないクロックを無効化する必要はありません
    void clockEnable(){
        __HAL_RCC_SYSCFG_CLK_ENABLE();
        __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_PWR_CLK_ENABLE();
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_USART6_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
        __HAL_RCC_CAN2_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
         __HAL_RCC_DMA2_CLK_ENABLE();
    }

public:
    uint16_t can_id;
    Button* buttons[2];
    Led* leds[2];
    Timer* tims[2];
    Pwm* pwms[8];

    // ボード初期化用コンストラクタ
    Board(uint16_t can_id_base){
        // CAN ID定義
        can_id = can_id_base + CAN_CHILD_ID;
        // システム初期化
        HAL_Init();
        SystemClockConfig();
        clockEnable();
        // 各種機能初期化
        leds[0] = new Led(GPIOA, GPIO_PIN_8);
        leds[1] = new Led(GPIOC, GPIO_PIN_9);
        buttons[0] = new Button(GPIOB, GPIO_PIN_14);
        buttons[1] = new Button(GPIOB, GPIO_PIN_15);
//         tims[0] = new Timer(TIM2, 31);
//         tims[1] = new Timer(TIM3, 31);
//         pwms[0] = new Pwm(tims[0], TIM_CHANNEL_1, GPIOA, GPIO_PIN_0);
//         pwms[1] = new Pwm(tims[0], TIM_CHANNEL_2, GPIOA, GPIO_PIN_1);
//         pwms[2] = new Pwm(tims[0], TIM_CHANNEL_3, GPIOA, GPIO_PIN_2);
//         pwms[3] = new Pwm(tims[0], TIM_CHANNEL_4, GPIOA, GPIO_PIN_3);
//         pwms[4] = new Pwm(tims[1], TIM_CHANNEL_2, GPIOA, GPIO_PIN_4);
//         pwms[5] = new Pwm(tims[1], TIM_CHANNEL_1, GPIOA, GPIO_PIN_6);
//         pwms[6] = new Pwm(tims[1], TIM_CHANNEL_3, GPIOB, GPIO_PIN_0);
//         pwms[7] = new Pwm(tims[1], TIM_CHANNEL_4, GPIOB, GPIO_PIN_1);
//         // インターフェイスの初期化
#ifdef UART6
        GlobalInterface::debug_port.init();
#endif
#ifdef UART1DMA
        GlobalInterface::dma_uart.init();
#endif
#ifdef CAN1
        GlobalInterface::can1.init(leds[0], leds[1]);
#endif
#ifdef CAN2
        GlobalInterface::can2.init(leds[0], leds[1]);
#endif

    }
    
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