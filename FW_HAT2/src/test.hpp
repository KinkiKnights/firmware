#pragma
#include "manage/manage.hpp"
#include "board.hpp"
namespace Test
{
    uint8_t test_serial[100];
    uint8_t test_param[4];
    uint16_t test_mode_counter = 0;
    constexpr uint16_t TEST_MODE_COUNTMS = 300;

    bool updateInit(Board& board,  uint8_t update_ms){
        if (board.buttons[0]->getState()
        && board.buttons[1]->getState()
        && test_mode_counter <= TEST_MODE_COUNTMS)
        {
            test_mode_counter += update_ms;
        }
        return (test_mode_counter > TEST_MODE_COUNTMS);
    }

    void updateGm6020(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        // GM6020の確認
        GM6020::Serial encoder;
        encoder.port_num = 7;
        uint16_t output = 0;
        // GM6020制御
        if (board.buttons[0]->getState()) output = 5000;
        if (board.buttons[1]->getState()) output = -5000;
        for (uint8_t i = 0; i < 7; i++){
            encoder.port[i] = i+1;
            if (i == 2 || i == 1)
                encoder.target[i] = output;
            else
                encoder.target[i] = -output;
        }
        encoder.encode(test_serial);
        manager.rcvCommand(test_serial);
    }

    void updatePwmServo(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        // サーボの確認
        PwmServo::Serial encoder;
        encoder.port_num = 8;
        uint16_t output = 125;
        // サーボ制御
        if (board.buttons[0]->getState()) output = 80;
        if (board.buttons[1]->getState()) output = 200;
        for (uint8_t i = 0; i < 8; i++){
            encoder.port[i] = i;
            encoder.spd[i] = 15;
            encoder.pos[i] = output;
        }
        encoder.encode(test_serial);
        manager.rcvCommand(test_serial);
    
    }

    void update(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        if (!updateInit(board,update_ms))return;

        // GM6020デバッグ
        // updateGm6020(board, manager, update_ms);
        // サーボモータ
        updatePwmServo(board, manager, update_ms);

    }
} // namespace Test
