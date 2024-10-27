#pragma once
#include "utils/can/epb.h"
#include "utils/can/live.h"
#include "utils/can/led.h"
#include "utils/can/servo.h"

const uint16_t CANID_EPB_STATUS = 0x00;
const uint16_t CANID_EPB_CMD = 0x01;
const uint16_t CANID_LIVE_REQUEST_ID = 0x10;
const uint16_t CANID_LIVE_RESPONSE_ID = 0x11;
// PWMサーボ
const uint16_t CANID_PWM_BASE_ID = 0x300;
const uint16_t CANID_PWM_FB_OFFSET = 0x10; // OFFSET
// ICSサーボ
const uint16_t CANID_ICS_BASE_ID = 0x380;
const uint16_t CANID_ICS_FB_OFFSET = 0x10; // OFFSET
// LEDテープ
const uint16_t CANID_LED_BASE_ID = 0x500;
// MD
const uint16_t CANID_MD_BASE_ID = 0x100;
const uint16_t CANID_MD_SETTING = 0x110;
const uint16_t CANID_MD_ENCODER = 0x120;
const uint16_t CANID_MD_FEEDBACK = 0x130;