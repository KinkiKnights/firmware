#include <cstdint> 
#include <stdio.h> 

class SPIPort{
public:
    struct Msg
    {
        uint8_t dlc;
        uint8_t* frame;
    };
    
public:
    // ループ処理(割り込みのみで良いなら省く)
    uint8_t update(){
        printf("dumy_transac:spi\n");
    }

    // 0で成功
    uint8_t send(Msg& msg){
        printf("spi0 :dlc=>%d [\n", msg.dlc);
        for (uint16_t idx = 0; idx < msg.dlc; idx++)
          printf("%x", msg.frame[idx]);
        printf("]\n");
    };

    // 返り値負でトラブル
    // 返り値正で受信バッファ現在数
    int16_t receive(Msg& msg){
        // いい受け取り方募集中！
    }
};

/**
 * MiddleWareを実装して、get時に処理することでロガー等に活用できないか？
 * どのみちCANの統括処理をする上位のモジュールが発生するだろうから不要かも？
 */
