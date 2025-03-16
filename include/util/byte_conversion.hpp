#pragma once
#include <stdint.h>
#include <cmath>

namespace Command
{
    void float_to_array(uint8_t *array, float value){
        union {
            float f;
            uint8_t bytes[4];
        } converter;
        converter.f = value;
        for (int i = 0; i < 4; ++i) {
            array[i] = converter.bytes[i];
        }
    }
    void array_to_float(uint8_t *array, float& value){
        union {
            float f;
            uint8_t bytes[4];
        } converter;
        for (int i = 0; i < 4; ++i) {
            converter.bytes[i] = array[i];
        }
        value = converter.f;
    }

    void uint16_to_array(uint8_t *array, uint32_t value){
        array[0] = value & 0xFF;
        array[1] = (value>>8) & 0xFF;
    }
    void array_to_uint16(uint8_t *array, uint32_t value){
        value = array[1];
        value = (value<<8) + array[0];
    }
    
    void int32_to_array(uint8_t *array, int32_t value){
        uint32_t val = std::abs(value);
        array[0] = val & 0xFF;
        array[1] = (val >> 8) & 0xFF;
        array[2] = (val >> 16) & 0xFF;
        array[3] = (val >> 24) & 0b01111111;
        if (value < 0){
            array[3] |= 0b10000000;
        }
    }
    void array_to_int32(uint8_t *array, int32_t& value){
        uint32_t val = array[3] & 0b01111111;
        val = (val << 8) + array[2];
        val = (val << 8) + array[1];
        val = (val << 8) + array[0];
        // フラグONなら負
        if (0b10000000 &  array[3])
            value = -val;
        else
            value = val;
    }

    void uint32_to_array(uint8_t *array, uint32_t value){
        array[0] = value & 0xFF;
        array[1] = (value >> 8) & 0xFF;
        array[2] = (value >> 16) & 0xFF;
        array[3] = (value >> 24) & 0xFF;
    }
    void array_to_uint32(uint8_t *array, uint32_t& value){
        value = array[3];
        value = (value << 8) + array[2];
        value = (value << 8) + array[1];
        value = (value << 8) + array[0];
    }
    
    void int24_plus_to_array(uint8_t *array, int32_t value, uint8_t second_7bit = 0){
        if(value >= 0)
            array[0] = 1;
        else
            array[0] = 0;
        uint32_t val_24 = static_cast<uint32_t>(std::abs(value)) & 0xFFFFFF;
        array[0] += (second_7bit << 1);
        array[1] = val_24 & 0xFF;
        array[2] = (val_24 >> 8) & 0xFF;
        array[3] = (val_24 >> 16) & 0xFF;
    }
    void array_to_int24_plus(uint8_t *array, int32_t& value){
        value = 0;
        for (uint8_t i = 0; i < 3; i++){
            value = value << 8;
            value += array[3 - i];
        }
        value *= ((array[0] & 0b1) == 1) ? 1: -1;
    }
    void array_to_int24_plus(uint8_t *array, int32_t& value, uint8_t& second_7bit){
        array_to_int24_plus(array, value);
        second_7bit = array[0] >> 1;
    }


} // namespace Command
