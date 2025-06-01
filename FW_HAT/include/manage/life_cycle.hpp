#pragma once
#define F4_CPU
#include "../../include/dev/F4/interface.hpp"
#include "../../include/protocol/can/can_msg.hpp"

namespace BoardManager
{
    // ボードの死活監視
    class BoardLifeCycle{
        const uint32_t CONNECT_TIMEOUT_MS = 5000;
        uint32_t last_live_ms;
        
    public:
        BoardLifeCycle(){
            last_live_ms = CONNECT_TIMEOUT_MS;
        }

        // 内部カウンタを更新したくない場合
        bool get(){
            if (last_live_ms > CONNECT_TIMEOUT_MS) return false;
            return true;
        }

        bool update(uint32_t update_ms){
            if (last_live_ms <= CONNECT_TIMEOUT_MS) last_live_ms += update_ms;
            return get();
        }

        void set(){
            last_live_ms = 0;
        }
    };
    // ボードの死活監視
    class BoardLifeCycleH{
        const uint32_t CONNECT_TIMEOUT_MS = 500;
        uint32_t last_live_ms;
        
    public:
        BoardLifeCycleH(){
            last_live_ms = CONNECT_TIMEOUT_MS;
        }

        // 内部カウンタを更新したくない場合
        bool get(){
            if (last_live_ms > CONNECT_TIMEOUT_MS) return false;
            return true;
        }

        bool update(uint32_t update_ms){
            if (last_live_ms <= CONNECT_TIMEOUT_MS) last_live_ms += update_ms;
            return get();
        }

        void set(){
            last_live_ms = 0;
        }
    };

    class BoardLifeCycleW{
        const uint32_t CONNECT_TIMEOUT_MS = 5000;
        uint32_t last_live_ms[2];
        
    public:
        BoardLifeCycleW()
        {
            last_live_ms[0] = CONNECT_TIMEOUT_MS;
            last_live_ms[1] = CONNECT_TIMEOUT_MS;
        }

        // 内部カウンタを更新したくない場合
        bool get(){
            if (last_live_ms[0] > CONNECT_TIMEOUT_MS) return false;
            if (last_live_ms[1] > CONNECT_TIMEOUT_MS) return false;
            return true;
        }

        bool update(uint32_t update_ms){
            if (last_live_ms[0] <= CONNECT_TIMEOUT_MS) last_live_ms[0] += update_ms;
            if (last_live_ms[1] <= CONNECT_TIMEOUT_MS) last_live_ms[1] += update_ms;
            return get();
        }

        void set(uint8_t idx = 0){
            last_live_ms[idx] = 0;
        }
    };

}

