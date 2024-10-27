#include <cstdint> 
#include <stdio.h> 

class CanPort{
public:
    struct Msg
    {
        uint8_t port;
        uint16_t can_id;
        uint8_t dlc;
        uint8_t frame[8];
    };
    
public:
    // ループ処理(割り込みのみで良いなら省く)
    uint8_t update(){
        printf("dumy_transac:can\n");
    }

    // 0で成功
    uint8_t send(Msg& msg){
        printf("can%d :ID=>%d [%x,%x,%x,%x,%x,%x,%x,%x]\n", msg.port, msg.can_id, 
        msg.frame[0], msg.frame[1], msg.frame[2], msg.frame[3], 
        msg.frame[4], msg.frame[5], msg.frame[6], msg.frame[7]);
    };

    // 返り値負でトラブル
    // 返り値正で受信バッファ現在数
    int16_t receive(Msg& msg){
        scanf("can%d=>%d[%x,%x,%x,%x,%x,%x,%x,%x]", msg.port, msg.can_id, 
        msg.frame[0], msg.frame[1], msg.frame[2], msg.frame[3], 
        msg.frame[4], msg.frame[5], msg.frame[6], msg.frame[7]);
    }
};

/**
 * MiddleWareを実装して、get時に処理することでロガー等に活用できないか？
 * どのみちCANの統括処理をする上位のモジュールが発生するだろうから不要かも？
 */
