#pragma once
#include <stdint.h>

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
    void int32_to_array(uint8_t *array, int32_t value){
        array[0] = value & 0xFF;
        array[1] = (value >> 8) & 0xFF;
        array[2] = (value >> 16) & 0xFF;
        array[3] = (value >> 24) & 0xFF;
    }
    void array_to_int32(uint8_t *array, int32_t& value){
        value = array[3];
        value = (value << 8) + array[2];
        value = (value << 8) + array[1];
        value = (value << 8) + array[0];
    }

} // namespace Command
