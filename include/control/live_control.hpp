#include "../protocol/_protocol.hpp"
#ifdef F4_CPU
#ifdef F4MD
#include "../dev/F4/interface_md.hpp"
#else
#include "../dev/F4/interface.hpp"
#endif
#else
#include "../dev/F3/interface.hpp"
#endif

class LiveControl
{
private:
    uint16_t counter;
    uint16_t counter_target;
    const uint16_t SEND_TERM = 3000;
    const uint16_t serial = 0;

    CanMessage msg;
    Can* can;
    LivingMessage encoder;
public:
    /**
     * @param call_ms 呼び出し頻度を設定します.
     * @param _can_id PINGフレームに設定する自身のIDです.
     */
    LiveControl(uint16_t call_ms, uint16_t _can_id, Can* _can){
        counter_target = SEND_TERM / call_ms;
        counter = counter_target * (0b11 & _can_id) / 0x4;
        can = _can;
        encoder.can_id = _can_id;
        encoder.serial_id = BUILD_UNIQU_TOKEN;
    }

    /**
     * @brief 呼び出し頻度に基づいて呼び出します。
     * @note　3秒に一度PINGを送信します。
     */
    void update(uint32_t loop_time, bool is_power = true, bool is_err = false){
        if(++counter < counter_target) return;
        // 更新処理
        encoder.run_time = HAL_GetTick() / 1000;
        encoder.setFlag(3, is_power);
        encoder.setFlag(4, is_err);
        encoder.loop_time = (loop_time < 200)? loop_time: 0x200;

        // フレーム送信処理
        msg = encoder.encode();
        can->send(msg);
        counter = 0;
    }
};
