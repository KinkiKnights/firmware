#pragma once
#include <stdio.h>
/*==========================================================
 * ============ CAN メッセージ定義
  ==========================================================*/

struct CanMessage
{
    uint8_t port;
    uint16_t id;
    uint8_t dlc;
    uint32_t filt;
    uint8_t data[8];
};