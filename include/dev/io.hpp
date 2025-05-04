#pragma once
#ifdef F4_CPU
#include <stm32f4xx_hal.h>
#else
#include <stm32f3xx_hal.h>
#endif
/*==========================================================
 * ============ ボタン
  ==========================================================*/
class Button{
    GPIO_TypeDef* gpio_group;
    uint32_t gpio_pin;
    bool is_last = false;
private:
    void init(){
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = gpio_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(gpio_group, &GPIO_InitStruct);
    }

public:
    Button(GPIO_TypeDef* _gpio_group, uint32_t _gpio_pin)
    : gpio_group(_gpio_group)
    , gpio_pin(_gpio_pin){init();}

    bool getState(){
        return (0 == HAL_GPIO_ReadPin(gpio_group, gpio_pin));
    }

    bool isReleased(){
        bool prev = is_last;
        is_last = getState();
        return (!is_last && prev);
    }

    bool isPush(){
        bool prev = is_last;
        is_last = getState();
        return (is_last && !prev);
    }
};

using GpioIN = Button;

/*==========================================================
 * ============ LED
  ==========================================================*/

class Led{
public:
    enum class LED_MODE{
        DEFAULT,
        CAN_LED,
        MODE,
        DEBUG
    };

private:
    GPIO_TypeDef* gpio_group;
    uint32_t gpio_pin;
    bool status;
    uint8_t flash_count = 0;
    LED_MODE mode = LED_MODE::DEFAULT;
    
private:
    void init(uint8_t pin_mode){
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = gpio_pin;
        if (pin_mode == 1)
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        else
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(gpio_group, &GPIO_InitStruct);
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_RESET);
    }

public:
    Led(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin, uint8_t pin_mode = 0)
    : gpio_group(_gpio_group)
    , gpio_pin(_gpio_pin)
    , status(false) {init(pin_mode);}

    void on(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        status = true;
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_SET);
    }

    void off(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        status = false;
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_RESET);
    }
    
    void toggle(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        if (status)
            off();
        else
            on();
    }

    void flash(uint8_t count, LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        if(++flash_count >= count){
            toggle();
            flash_count = 0;
        }
    }

    // モード優先設定
    void setMode(LED_MODE _mode){
        mode = _mode;
    }
};
    

using GpioOut = Led;

/*==========================================================
 * ============ 共通タイマ
  ==========================================================*/
class Timer{
    TIM_HandleTypeDef htim;
    uint16_t prescaler;
    uint16_t period;

private:
    void init(TIM_TypeDef* tim){
        // タイマーの基本設定
        htim.Instance = tim;
        htim.Init.Prescaler = prescaler;
        htim.Init.Period = period;
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
        htim.Init.RepetitionCounter = 0;
        if (HAL_TIM_Base_Init(&htim) != HAL_OK)Error_Handler();
#ifdef F4_CPU
        if (tim == TIM1 || tim == TIM2 || tim == TIM3 || tim == TIM8 || tim == TIM12){
#endif
#ifndef F4_CPU
        if (tim == TIM1 || tim == TIM2 || tim == TIM3){
#endif
            // 設定インスタンス作成
            TIM_ClockConfigTypeDef sClockSourceConfig = {0};
            TIM_MasterConfigTypeDef sMasterConfig = {0};
            // クロック源設定
            sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
            if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK)Error_Handler();
            if (HAL_TIM_PWM_Init(&htim) != HAL_OK)Error_Handler();
            // マスター設定
            sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
#ifndef F4_CPU
            sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
#endif
            sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
            if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)Error_Handler();        
        }
#ifdef F4_CPU
        if (tim == TIM1 || tim == TIM8){
#endif
#ifndef F4_CPU
        if (tim == TIM1){
#endif
        
            TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfi = {0};
            sBreakDeadTimeConfi.OffStateRunMode = TIM_OSSR_DISABLE;
            sBreakDeadTimeConfi.OffStateIDLEMode = TIM_OSSI_DISABLE;
            sBreakDeadTimeConfi.LockLevel = TIM_LOCKLEVEL_OFF;
            sBreakDeadTimeConfi.DeadTime = 0;
            sBreakDeadTimeConfi.BreakState = TIM_BREAK_DISABLE;
            sBreakDeadTimeConfi.BreakPolarity = TIM_BREAKPOLARITY_LOW;
            sBreakDeadTimeConfi.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
            if (HAL_TIMEx_ConfigBreakDeadTime(&htim, &sBreakDeadTimeConfi) != HAL_OK)Error_Handler();  
#ifndef F4_CPU
            __HAL_TIM_MOE_ENABLE(&htim);
            HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
#endif
        }
    };

public:
    Timer(TIM_TypeDef* tim, uint16_t _prescaler, uint16_t _period = 9999)
    : prescaler(_prescaler), period(_period){
        init(tim);
    }
    
    TIM_HandleTypeDef* getHtim(){
        return &htim;
    }

    uint16_t getPrescaler(){
        return prescaler;
    }
};



/*==========================================================
 * ============ PWM出力
  ==========================================================*/
class Pwm{
    TIM_HandleTypeDef* htim;
    uint32_t channel;
    bool is_ex;

private:
    void init(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
        // PWMモードの選択・設定
        TIM_OC_InitTypeDef sConfigOC = {0};
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
#ifndef F4_CPU
        if (is_ex)
            sConfigOC.OCMode = TIM_OCMODE_ASSYMETRIC_PWM1;
#endif
        sConfigOC.Pulse = 0;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        if (htim->Instance == TIM1){
            sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
            sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
            sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
        }
        bool flag_success = (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel) == HAL_OK);
        while(!flag_success){};
        
        // 出力ピン設定
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = _gpio_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        if (htim->Instance == TIM2){
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        }else if (htim->Instance == TIM3){
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        }
#ifdef F4_CPU
        else if (htim->Instance == TIM8){
            GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
        }
        else if (htim->Instance == TIM12){
            GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
        }
#endif
#ifndef F4_CPU
        else if (htim->Instance == TIM15){
            GPIO_InitStruct.Alternate = GPIO_AF9_TIM15;
        }else if (htim->Instance == TIM16){
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM16;
        }else if (htim->Instance == TIM1){
            GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;
        }
#endif
        HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
        // Duty比 初期化
        setPeriod(0);
        // PWM出力開始
        HAL_TIM_PWM_Start(htim, channel);
        if (htim->Instance == TIM1){
            HAL_TIMEx_PWMN_Start(htim, channel);
        }
    }

public:
    Pwm(Timer* _tim, uint32_t _channel,GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin, bool _is_ex = false)
    : htim(_tim->getHtim())
    , channel(_channel)
    , is_ex(_is_ex){
        init(_gpio_group, _gpio_pin);
    }

    void setPeriod(uint16_t on_period){
        __HAL_TIM_SET_COMPARE(htim, channel, on_period);
    }

    void setDuty(float duty, uint16_t counta = 9999){
        if (duty < 0.f) duty = 0;
        if (duty > 1.f) duty = 1.f;
        if (is_ex)
            setPeriod(counta - static_cast<uint16_t>(counta * duty));
        else
            setPeriod(static_cast<uint16_t>(counta * duty));
    }

};

/*==========================================================
 * ============ エンコーダ用タイマ TIM2・TIM3のみ
  ==========================================================*/
class EncoderTimer{
    TIM_HandleTypeDef htim;
    uint16_t prescaler;
    uint16_t period;

private:
    void initTimer(TIM_TypeDef* tim){
        // タイマーの基本設定
        htim.Instance = tim;
        htim.Init.Prescaler = 0;
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.Period = 0xFFFF;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&htim) != HAL_OK)Error_Handler();

        TIM_Encoder_InitTypeDef sConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
            sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
        sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
        sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
        sConfig.IC1Filter = 0;
        sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
        sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
        sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
        sConfig.IC2Filter = 0;
        if (HAL_TIM_Encoder_Init(&htim, &sConfig) != HAL_OK)Error_Handler();
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)Error_Handler();    
        HAL_TIM_Encoder_Start(&htim, TIM_CHANNEL_ALL); 
    }
    
    void initGpio(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = _gpio_pin;
        if (htim.Instance == TIM2){
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        }else if (htim.Instance == TIM3){
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        }
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
    }

public:
    EncoderTimer(TIM_TypeDef* tim, GPIO_TypeDef *_gpio_group1, uint32_t _gpio_pin1, GPIO_TypeDef *_gpio_group2, uint32_t _gpio_pin2)
    {
        initTimer(tim);
        initGpio(_gpio_group1, _gpio_pin1);
        initGpio(_gpio_group2, _gpio_pin2);
    }
    
    uint32_t getCounter(){
        return __HAL_TIM_GET_COUNTER(&htim);
    }
};


/*==========================================================
 * ============ パルスカウント用タイマ TIM2・TIM3のみ
  ==========================================================*/
class Encoder1InputTimer{
    TIM_HandleTypeDef htim;
    uint16_t prescaler;
    uint16_t period;

private:
    void initTimer(TIM_TypeDef* tim, uint32_t _channel){
        // タイマーの基本設定
        htim.Instance = tim;
        htim.Init.Prescaler = 0;
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.Period = 0xFFFF;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&htim) != HAL_OK)Error_Handler();

        TIM_IC_InitTypeDef sConfigIC = {0};
        sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
        sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
        sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
        sConfigIC.ICFilter = 0;
        if (HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, _channel) != HAL_OK)Error_Handler();

        TIM_MasterConfigTypeDef sMasterConfig = {0};
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)Error_Handler();    
        HAL_TIM_IC_Start(&htim, _channel); 
    }
    
    void initGpio(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = _gpio_pin;
        if (htim.Instance == TIM2){
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        }else if (htim.Instance == TIM3){
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        }
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
    }

public:
    Encoder1InputTimer(TIM_TypeDef* tim, uint32_t _channel, GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin)
    {
        initTimer(tim, _channel);
        initGpio(_gpio_group, _gpio_pin);
    }
    
    uint32_t getCounter(){
        return __HAL_TIM_GET_COUNTER(&htim);
    }
};