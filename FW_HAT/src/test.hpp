#pragma
#include "manage/manage.hpp"
#include "board.hpp"
namespace Test
{
    uint8_t test_serial[100];
    uint8_t test_param[4];
    uint16_t test_mode_counter = 0;
    constexpr uint16_t TEST_MODE_COUNTMS = 300;
    bool test_mode = false;
    uint8_t test_version = 0;

    bool updateInit(Board& board,  uint8_t update_ms){
        if (board.buttons[0]->getState()
        && board.buttons[1]->getState()
        && test_mode_counter <= TEST_MODE_COUNTMS)
        {
            test_mode_counter += update_ms;
        }

        // テストモード開始シーケンス
        if ((test_mode_counter > TEST_MODE_COUNTMS) && !test_mode){
            // ボタン離すまで待機
            while (board.buttons[0]->getState() || board.buttons[1]->getState()){
                HAL_Delay(500);
                board.leds[0]->toggle();
            }

            while (!board.buttons[0]->isPush())
            {
                if (board.buttons[1]->isPush())
                    test_version = (test_version + 1) % 3;

                board.leds[0]->off();
                HAL_Delay(1000);
                for (uint8_t i = 0; i < (test_version + 1);i++){
                    board.leds[0]->on();
                    HAL_Delay(100);
                    board.leds[0]->off();
                    HAL_Delay(100);
                }
            }

            while (board.buttons[0]->getState() || board.buttons[1]->getState()){
                HAL_Delay(50);
                board.leds[0]->toggle();
            }
            test_mode = true;
        }

        return test_mode;
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

    const uint16_t MAX_COUNT = 1000;
    const uint16_t MIN_COUNT = 500;
    const uint16_t PLS_DIFF = 5;
    uint16_t target_count = (MIN_COUNT + MAX_COUNT)/2;
    void updatePwmServo(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        // 位置の計算
        if(board.buttons[0]->getState())
            target_count += PLS_DIFF;
        if(board.buttons[1]->getState())
            target_count -= PLS_DIFF;
        if (target_count > MAX_COUNT) target_count = MAX_COUNT;
        if (target_count < MIN_COUNT) target_count = MIN_COUNT;
        
        // サーボ制御
        PwmServo::Serial encoder;
        encoder.port_num = 8;
        for (uint8_t i = 0; i < 8; i++){
            encoder.port[i] = i;
            encoder.spd[i] = 15;
            encoder.pos[i] = target_count;
        }
        encoder.encode(test_serial);
        manager.rcvCommand(test_serial);
        printf("Test Target Count: %d\n", target_count);
    }

    const float TEST_DUTY = 0.5f;
    void updateMotor(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        // 位置の計算
        float output = 0.f;
        if(board.buttons[0]->getState())
            output = TEST_DUTY;
        if(board.buttons[1]->getState())
            output = -TEST_DUTY;
        
        // サーボ制御
        Motor::Serial encoder;
        encoder.port_num = 8;
        for (uint8_t i = 0; i < 8; i++){
            encoder.port[i] = i;
            encoder.duty[i] = static_cast<int16_t>(output * 0x7FFF);
            if (encoder.duty[i] > 0x7FFF) encoder.duty[i] = 0x7FFF;
            if (encoder.duty[i] < -0x7FFF) encoder.duty[i] = -0x7FFF;
        }
        encoder.encode(test_serial);
        manager.rcvCommand(test_serial);
        printf("Test Output Duty: %f (%d)\n", output, encoder.duty[0]);
    }

    void update(Board& board,BoardManager::BoardManager& manager, uint8_t update_ms){
        if (!updateInit(board,update_ms))return;
        
        // GM6020デバッグ
        if (test_version == 0)
            updateGm6020(board, manager, update_ms);
        // サーボモータ
        if (test_version == 1)
            updatePwmServo(board, manager, update_ms);
        // モータ
        if (test_version == 2)
            updateMotor(board, manager, update_ms);
    }
} // namespace Test
