#pragma once
#include <stdint.h>
#include <stdlib.h>
#define bool uint8_t
#define true 1
#define false 0

union Float32 {
    float f;
    uint8_t u[4];
};

// int32_t型の値をバイト配列にシリアライズする関数
void serializeInt32(int32_t value, uint8_t *lsb) {
    uint32_t value_abs = abs(value);
    for (uint8_t i = 0; i < 4; i++) {
        lsb[i] = (value_abs >> (i * 8)) & 0xff;
    }
    if (value >= 0)
        lsb[3] &= 0x7f;
    else
        lsb[3] |= 0x80;
}

// バイト配列からint32_t型の値をデシリアライズする関数
int32_t deserializeInt32(const uint8_t *lsb) {
    int32_t value = 0;
    for (uint8_t i = 0; i < 4; i++) {
        value |= ((int32_t)lsb[i]) << (i * 8);
    }
    if (lsb[3] & 0x80)
        value = -value;
    return value;
}


// 2バイトで浮動小数点数をシリアライズする関数
void serializeFloatHalf(float value, uint8_t *lsb) {
    uint16_t data = 0;
    uint8_t exponent = 0;
    if (value < 0) {
        data |= (1 << 15);
        value = -value;
    }
    if (value >= 1) {
        while (value >= 10.f && exponent < 7) {
            value *= 0.1f;
            exponent++;
        }
    } else if (value > 0) {
        data |= (1 << 14);
        while (value < 1.f && exponent < 7) {
            value *= 10.f;
            exponent++;
        }
    }
    data |= (exponent << 10);
    data |= (uint16_t)(value * 100);
    lsb[0] = (uint8_t)(data & 0xFF);
    lsb[1] = (uint8_t)(data >> 8);
}

// 2バイトのデータから浮動小数点数をデシリアライズする関数
float deserializeFloatHalf(uint8_t *lsb) {
    uint16_t data = lsb[0] + ((uint16_t)lsb[1] << 8);
    if (data == 0xFFFF) return 0;
    float value = (float)(data & 0x3FF) * 0.01f;
    if (data & (1 << 15)) value = -value;
    float multiplier = (data & (1 << 14)) ? 0.1f : 10.f;
    uint8_t exponent = (data >> 10) & 0x07;
    for (uint8_t i = 0; i < exponent; i++) {
        value *= multiplier;
    }
    return value;
}


// 4バイトで浮動小数点数をシリアライズする関数
void serializeFloatFull(float value, uint8_t *lsb) {
    
    union Float32 u;
    u.f = value;
    for (int i = 0; i < 4; i++) {
        lsb[i] = u.u[i];
    }
}

// 4バイトのデータから浮動小数点数をデシリアライズする関数
float deserializeFloatFull(uint8_t* lsb) {

    union Float32 u;
    for (int i = 0; i < 4; i++) {
        u.u[i] = lsb[i];
    }
    return u.f;
}