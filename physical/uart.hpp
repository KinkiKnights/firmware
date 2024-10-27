#include <cstdint> 
#include <stdio.h> 
#include <string> 

class UartPort{
public:
    
public:
    // ループ処理(割り込みのみで良いなら省く)
    std::string update(){
        printf("dumy_transac:uart\n");
    }

    // 0で成功
    uint8_t send(std::string msg){
        printf(msg.c_str());
    };

    // 返り値負でトラブル
    // 返り値正で受信バッファ現在数
    int16_t receive(std::string& msg){
        // いい受け取り方募集中！
    }
};