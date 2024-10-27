#pragma once
#include <stm32f3xx_hal.h>
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
        return (1 == HAL_GPIO_ReadPin(gpio_group, gpio_pin));
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
        MODE
    };

private:
    GPIO_TypeDef* gpio_group;
    uint32_t gpio_pin;
    bool status;
    uint8_t flash_count = 0;
    LED_MODE mode = LED_MODE::DEFAULT;
    
private:
    void init(){
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = gpio_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(gpio_group, &GPIO_InitStruct);
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_RESET);
    }

public:
    Led(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin)
    : gpio_group(_gpio_group)
    , gpio_pin(_gpio_pin)
    , status(false) {init();}

    void on(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        status = true;
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_SET);
    }

    void off(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        status = true;
        HAL_GPIO_WritePin(gpio_group, gpio_pin, GPIO_PIN_RESET);
    }
    
    void toggle(LED_MODE _mode = LED_MODE::DEFAULT){
        if ((mode != LED_MODE::DEFAULT) && mode != _mode) return;
        if (status)
            off();
        if (status)
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

private:
    void init(TIM_TypeDef* tim){
        // 設定インスタンス作成
        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        
        // ハンドル設定
        htim.Instance = tim;
        htim.Init.Prescaler = prescaler;
        htim.Init.Period = 9999;
        htim.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
        if (HAL_TIM_Base_Init(&htim) != HAL_OK)Error_Handler();

        // クロック源設定
        sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
        if (HAL_TIM_ConfigClockSource(&htim, &sClockSourceConfig) != HAL_OK)Error_Handler();
        if (HAL_TIM_PWM_Init(&htim) != HAL_OK)Error_Handler();
        
        // マスター設定
        sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
        sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
        if (HAL_TIMEx_MasterConfigSynchronization(&htim, &sMasterConfig) != HAL_OK)Error_Handler();
    };

public:
    Timer(TIM_TypeDef* tim, uint16_t _prescaler)
    : prescaler(_prescaler){
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

private:
    void init(GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin){
        // PWMモードの選択・設定
        TIM_OC_InitTypeDef sConfigOC = {0};
        sConfigOC.OCMode = TIM_OCMODE_PWM1;
        sConfigOC.Pulse = 0;
        sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
        sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
        bool flag_success; 
        switch (channel)
        {
        case TIM_CHANNEL_1:
            flag_success = (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_1) == HAL_OK);
            break;
        case TIM_CHANNEL_2:
            flag_success = (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_2) == HAL_OK);
            break;
        case TIM_CHANNEL_3:
            flag_success = (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_3) == HAL_OK);
            break;
        case TIM_CHANNEL_4:
            flag_success = (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, TIM_CHANNEL_4) == HAL_OK);
            break;
        default:
            flag_success = true;
            break;
        }
        if (!flag_success)Error_Handler();
        
        // 出力ピン設定
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = _gpio_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        if(htim->Instance == TIM2){
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        } else if(htim->Instance == TIM3){
            GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        }
        HAL_GPIO_Init(_gpio_group, &GPIO_InitStruct);
        
        // PWM波 初期化
        setPeriod(0);
        HAL_TIM_PWM_Start(htim, channel);
    }

public:
    Pwm(Timer* _tim, uint32_t _channel,GPIO_TypeDef *_gpio_group, uint32_t _gpio_pin)
    : htim(_tim->getHtim())
    , channel(_channel){
        init(_gpio_group, _gpio_pin);
    }

    void setPeriod(uint16_t on_period){
        __HAL_TIM_SET_COMPARE(htim, channel, on_period);
    }

    void setDuty(Pwm* pwm, float duty){
        if (duty < 0.f) duty = 0;
        if (duty > 0.99f) duty = 0.99f;
        setPeriod(static_cast<uint16_t>(9999 * duty));
    }

};