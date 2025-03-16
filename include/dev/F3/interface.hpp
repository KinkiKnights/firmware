#pragma once
#include <stm32f3xx_hal.h>
#include <stm32f3xx_hal_uart.h>
#include <string.h>
#include <stdio.h>
#include "../io.hpp"
#include "../../protocol/can_msg.hpp"

/*==========================================================
 * ============ Uart インターフェイス
 * =================初期化のタイミングからコンストラクタ外でinit
  ==========================================================*/
class Uart{
    UART_HandleTypeDef huart;
    USART_TypeDef* port;
public:
    void init(){
        // 入出力ピンの設定
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        // 複数使用の場合は下記を変更
        GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // UART通信の設定
        huart.Instance = port;
        huart.Init.BaudRate = 115200;
        huart.Init.WordLength = UART_WORDLENGTH_8B;
        huart.Init.StopBits = UART_STOPBITS_1;
        huart.Init.Parity = UART_PARITY_NONE;
        huart.Init.Mode = UART_MODE_TX_RX;
        huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart.Init.OverSampling = UART_OVERSAMPLING_16;
        huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
        HAL_UART_Init(&huart);    
        // printf関数のための初期化
        setbuf(stdout, NULL);
    }

    Uart(USART_TypeDef* _port)
    :port(_port){}

    UART_HandleTypeDef* getHuart(){
       return &huart;
    }

    void send(char* str){
        HAL_UART_Transmit(&huart, (uint8_t*)str, strlen(str), 1000);
    }
};

#define DMA_WRITE_PTR ( (USART_RX_BUFFSIZE - __HAL_DMA_GET_COUNTER(huart.hdmarx)) % (USART_RX_BUFFSIZE) )
class UartDMA{
    UART_HandleTypeDef huart;
    USART_TypeDef* port;
    static const uint16_t USART_RX_BUFFSIZE = 1280;
    uint8_t RxBuff[USART_RX_BUFFSIZE];
public:
    DMA_HandleTypeDef hdma_usart_rx;
    void init(){
        // 入出力ピンの設定
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        // 複数使用の場合は下記を変更
        if (port == USART1){
            GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
            GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        }
        if (port == USART2){
            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        }
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // DMA初期化
        if (port == USART1){
            HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
            hdma_usart_rx.Instance = DMA1_Channel5;
        }
        if (port == USART2){
            HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
            hdma_usart_rx.Instance = DMA1_Channel6;
        }
        hdma_usart_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart_rx.Init.Mode = DMA_CIRCULAR;
        memset(RxBuff, 0, sizeof(RxBuff));
        if (HAL_DMA_Init(&hdma_usart_rx) != HAL_OK) {printf("DMA受信初期化失敗");}
        __HAL_LINKDMA(&huart,hdmarx,hdma_usart_rx);   

        // UART通信の設定
        huart.Instance = port;
        huart.Init.BaudRate = 115200;
        huart.Init.WordLength = UART_WORDLENGTH_8B;
        huart.Init.StopBits = UART_STOPBITS_1;
        huart.Init.Parity = UART_PARITY_NONE;
        if (port == USART1)
            huart.Init.Mode = UART_MODE_TX_RX;
        else
            huart.Init.Mode = UART_MODE_RX;
        huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart.Init.OverSampling = UART_OVERSAMPLING_16;
        huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
        huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
        HAL_UART_Init(&huart);
        if (port == USART1)
            setbuf(stdout, NULL);
        __HAL_UART_DISABLE_IT(&huart, UART_IT_PE);
        __HAL_UART_DISABLE_IT(&huart, UART_IT_ERR);
        HAL_UART_Receive_DMA(&huart, RxBuff, USART_RX_BUFFSIZE);
    }

    UartDMA(USART_TypeDef* _port)
    :port(_port){}

    UART_HandleTypeDef* getHuart(){
       return &huart;
    }

    void send(char* str){
        HAL_UART_Transmit(&huart, (uint8_t*)str, strlen(str), 1000);
    }
    public: // DMA処理関連
    uint32_t rd_ptr = 0;
    bool RX_IsEmpty(void)
    {
        return (rd_ptr == DMA_WRITE_PTR);
    }

    char RX_Read(void)
    {
        uint8_t c = 0;
        if(rd_ptr != DMA_WRITE_PTR) {
            c = RxBuff[rd_ptr++];
            rd_ptr %= USART_RX_BUFFSIZE;
        }
        return c;
    }
};

/*==========================================================
 * ============ 標準UARTポート定義
  ==========================================================*/
#ifdef UART1
namespace GlobalInterface{
    Uart debug_port(USART1);
}
extern "C" int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(GlobalInterface::debug_port.getHuart(),(uint8_t *)ptr,len,10);
  return len;
}
#endif

#ifdef UART1DMA
namespace GlobalInterface{
    UartDMA debug_port(USART1);
}

extern "C" int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(GlobalInterface::debug_port.getHuart(),(uint8_t *)ptr,len,10);
  return len;
}
extern "C" void DMA1_Channel5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&GlobalInterface::debug_port.hdma_usart_rx);
}
#endif
#ifdef UART2
namespace GlobalInterface{
    UartDMA dma_port(USART2);
}
extern "C" void DMA1_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&GlobalInterface::dma_port.hdma_usart_rx);
}

#endif

/*==========================================================
 * ============ CAN インターフェイス
 * =================初期化のタイミングからコンストラクタ外でinit
  ==========================================================*/
class Can{
public:
    CAN_HandleTypeDef hcan;
    Led* tx_led;
    Led* rx_led;

    void init(Led* _tx_led, Led* _rx_led){
        tx_led = _tx_led;
        rx_led = _rx_led;

        // CANの設定
        hcan.Instance = CAN;
        hcan.Init.Prescaler = 1;
        hcan.Init.Mode = CAN_MODE_NORMAL;
        hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
        hcan.Init.TimeSeg1 = CAN_BS1_13TQ;
        hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
        hcan.Init.TimeTriggeredMode = DISABLE;
        hcan.Init.AutoBusOff = DISABLE;
        hcan.Init.AutoWakeUp = DISABLE;
        hcan.Init.AutoRetransmission = DISABLE;
        hcan.Init.ReceiveFifoLocked = DISABLE;
        hcan.Init.TransmitFifoPriority = DISABLE;
        if (HAL_CAN_Init(&hcan) != HAL_OK){/*Error_Handler();*/}
        
        // GPIOの設定(各CAN固有)
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        if (hcan.Instance == CAN)
        {
            GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
            HAL_NVIC_SetPriority(CAN_RX0_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
        }

        // フィルタ設定(一旦貫通設定)
        CAN_FilterTypeDef can_filter;
        can_filter.FilterActivation = ENABLE;
        can_filter.FilterBank = 0;
        can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;
        can_filter.FilterIdHigh = 0;
        can_filter.FilterIdLow = 0;
        can_filter.FilterMaskIdHigh = 0;
        can_filter.FilterMaskIdLow = 0;
        can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
        can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
        can_filter.SlaveStartFilterBank = 14;
        if (HAL_CAN_ConfigFilter(&hcan, &can_filter) != HAL_OK){}

        // CAN通信初期化・開始
        HAL_CAN_Start(&hcan);
        HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
    }

    void send(CanMessage msg){
        uint32_t last_TxMailBox;
        CAN_TxHeaderTypeDef TxHeader;
        // メールボックス空き容量確認
        if(0 < HAL_CAN_GetTxMailboxesFreeLevel(&hcan)){
            TxHeader.StdId = msg.id;
            TxHeader.RTR = CAN_RTR_DATA;
            TxHeader.IDE = CAN_ID_STD;
            TxHeader.DLC = msg.dlc;
            TxHeader.TransmitGlobalTime = DISABLE;
            HAL_CAN_AddTxMessage(&hcan, &TxHeader, msg.data, &last_TxMailBox);
            // tx_led->flash(5, Led::LED_MODE::CAN_LED);
        }
    }
};


/*==========================================================
 * ============ CAN 軽量リングバッファ
  ==========================================================*/

// CANバッファ量定義
constexpr uint16_t CAN_BUFF_NUM = 100;
constexpr uint16_t CAN_BUFF_RW_MARGIN = 990;
// CANメッセージ受信バッファ
class CanBuffer{
    CanMessage buff[CAN_BUFF_NUM];
    uint16_t next_read = 0;
    uint16_t next_write = 0;
    uint16_t rw_diff = 0;
public:
    bool set(CanMessage& msg){
        if (rw_diff > CAN_BUFF_RW_MARGIN) return false;
        buff[next_write++] = msg;
        next_write = next_write % CAN_BUFF_NUM;
        rw_diff++;
        return true;
    }

    bool get(CanMessage& _msg){
        if (rw_diff < 1) return false;
        _msg = buff[next_read++];
        next_read = next_read % CAN_BUFF_NUM;
        rw_diff--;
        return true;
    }

};
// 共有CAN受信バッファ
namespace GlobalInterface{
    CanBuffer can_buff;
}
#define CAN1
/*==========================================================
 * ============ CAN 通信割り込み定義
  ==========================================================*/
#ifdef CAN1

// CAN1が有効な場合のインターフェイス
namespace GlobalInterface{
    Can can1;
}

// CAN1の受信時割り込み処理
extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;    
    CanMessage msg;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, msg.data) == HAL_OK){
        // GlobalInterface::can1.rx_led->flash(5, Led::LED_MODE::CAN_LED);
        msg.port = 0;
        msg.dlc = RxHeader.DLC;
        msg.id = RxHeader.StdId;
        msg.filt = RxHeader.FilterMatchIndex;
        GlobalInterface::can_buff.set(msg);
    }
}

// CAN1の受信ハンドラ定義
extern "C" void CAN_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&GlobalInterface::can1.hcan);
}
#endif
