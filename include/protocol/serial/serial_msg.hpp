#include "live.hpp"
#include "pwm.hpp"
#include "ics.hpp"
#include "gm6020.hpp"
#include "mtr.hpp"


#ifndef SERIAL_MSG
#define SERIAL_MSG
/*==========================================================
 * ============ CAN メッセージ定義
  ==========================================================*/
namespace SerialConvert
{
  inline uint16_t array_2_uint16(uint8_t *array){
    uint16_t val = array[0];
    return (val << 8) + array[1];
  }

  inline void uint16_2_array(uint16_t val, uint8_t *array){
    array[0] = val >> 8;
    array[1] = 0xFF & val;
  }

  inline int16_t array_2_int16(uint8_t *array){
    uint16_t val = array[0];
    return (int16_t)((val << 8) + array[1]);
  }

  inline void int16_2_array(int16_t val, uint8_t *array){
    array[0] = (uint8_t)((val >> 8) & 0xFF); 
    array[1] = (uint8_t)(val & 0xFF);
  }

  inline int32_t array_2_int32(uint8_t *array){
    uint32_t val = array[0];
    val = (val << 8) + array[1];
    val = (val << 8) + array[2];
    val = (val << 8) + array[3];
    return (int32_t)(val);
  }

  inline void int32_2_array(int32_t val, uint8_t *array){
    array[0] = (uint8_t)((val >> 24) & 0xFF); 
    array[1] = (uint8_t)((val >> 16) & 0xFF); 
    array[2] = (uint8_t)((val >> 8) & 0xFF); 
    array[3] = (uint8_t)(val & 0xFF);
  }

  inline void array_2_uint12_4(uint16_t& val, uint8_t& sub, uint8_t *array){
    // バリデーション
    val = ((array[0] & 0xF)<<8) + array[1];
    sub = array[0] >> 4;
  }

  inline void uint12_4_2_array(uint16_t val, uint8_t sub, uint8_t *array){
    // バリデーション
    if (sub > 0xF)sub = 0xF;
    if (val > 0xFFF)val = 0xFFF;

    array[0] = (val >> 8) + (sub << 4);
    array[1] = val & 0xFF;
  }


} // namespace SerialConvert

#endif