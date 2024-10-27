#include <cstdint>
#include <math.h>
#include "./frame.hpp"
/**
 * @brief CAN通信時のメッセージのエンコード、デコード機能を提供します
 * @note 単体テストに必要なデバッグ機能も提供します
 */

namespace CanMsg{

    /**
     * @brief 2ポートのDuty制御の機能のみを持った、簡易制御可能なMDです。
     * @note 基板による集中制御想定のため、ポート個別のメッセージは発生させない設計 リバースも同様
     */
    struct SimpleMotor{
    private:
        static const uint16_t BIT_15 = 0b0111111111111111;
        static const uint16_t BITMASK_15 = 0b1000000000000000;

    public:
        bool decode_err = false;
        uint16_t id = 0;
        float duty[2];

    public:
        /**
         * @param Duty値を入れた2要素の配列を渡します。
         */
        SimpleMotor(Frame* msg){
            decode_err = true;
            // バリデーション
            if (msg->dlc != 4) return;
            for(uint8_t idx = 0; idx < 2; idx++){

                uint16_t val = msg->frame[idx * 2] + msg->frame[idx * 2] * 0x100;
                float duty[idx] = 1.f * (BIT_15 & val) / BIT_15;
                if (BITMASK_15 & val)
                    duty[idx] *= -1.f;
            }
            id = msg->id;
            decode_err = false;
            return;
        }

        void encode(){
            Frame msg;
            for(uint8_t idx = 0; idx < 2; idx++){
                
                // バリデーション
                if (duty[idx] < -0.98f) duty[idx] = -0.98f;
                if (duty[idx] > 0.98f) duty[idx] = 0.98f;

                uint16_t percentage = abs(duty[idx] * BIT_15);
                if (duty < 0)
                    percentage |= BITMASK_15;
                msg.frame[idx * 2] = static_cast<uint8_t>(percentage & 0xff);
                msg.frame[idx * 2 + 1] = static_cast<uint8_t>((percentage >> 8) & 0xff);
            }
            msg.dlc = 4;
            msg.id = id;
        }
    };

}