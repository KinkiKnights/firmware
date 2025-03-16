#pragma once
#define F4_CPU
#include "../../include/protocol/_protocol.hpp"
#include "../../include/dev/F4/interface.hpp"

namespace BoardManager
{
    class Error{
        // トラブル情報
    public:
        static const uint8_t ERROR_NUM = 8;
        static const uint8_t ERROR_OVER_ID = 0;
        static const uint8_t ERROR_TIMEOUT = 1;
        static const uint8_t ERROR_TIMEOUT_BOARD = 2;
        static const uint8_t ERROR_POWEROFF = 3;
        static const uint8_t ERROR_CRITICAL = 4;
        uint8_t errorr_flag = 0b1;
    public:
        // エラーフラグのセット
        void set(uint8_t err_idx){
            errorr_flag |= 0b1 << err_idx;
        }
        // エラーフラグのクリア
        void reset(uint8_t err_idx){
            errorr_flag &= ~(0b1 << err_idx);
        }
        // エラー情報一括設定
        void rawSet(uint8_t _flag){
            errorr_flag = (_flag & ~0b11) + (errorr_flag & 0b11);
        }
        // エラー情報取得
        uint64_t get(){
            return errorr_flag;
        }
    };

    class BaseManager{
    public:
        const uint16_t CONNECT_TIMEOUT_MS = 10000;
        uint16_t last_live_ms;
        uint16_t serial_id = 0;
        uint16_t run_time_sec = 0;
        uint16_t loop_msec = 0;
        Error err;

        BaseManager() {
            last_live_ms = CONNECT_TIMEOUT_MS;
        }

        uint16_t getDebug(){
            return last_live_ms;
        }

        bool update(uint16_t update_ms){
            if (last_live_ms > CONNECT_TIMEOUT_MS){
                err.set(Error::ERROR_TIMEOUT);
                err.reset(Error::ERROR_OVER_ID);
                return false;
            }
            last_live_ms+= update_ms;
            return true;
        }
        
        void rcvLiving(LivingMessage& msg){
            // 多重接続排除(タイムアウト済みの場合は不問)
            if (last_live_ms <= CONNECT_TIMEOUT_MS && msg.serial_id != serial_id){
                err.set(Error::ERROR_OVER_ID);
                return;
            }
            // 基板情報更新
            last_live_ms = 0;
            run_time_sec = msg.run_time;
            serial_id = msg.serial_id;
            loop_msec = msg.loop_time;
            err.reset(Error::ERROR_TIMEOUT);
            err.rawSet(msg.status_flag);
        }
        
        
    };

}