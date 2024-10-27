#include <cstdint> 

class SPI{
    public:
        struct KVS{
            uint16_t id;
            float value;
        };

        struct CTR{
            uint8_t board_type;
            uint8_t control_type;
            uint8_t board_id;
            uint8_t target_port;
            float value;
        };

        struct CTRFB{
            uint8_t board_type;
            uint8_t feedback_type;
            uint8_t board_id;
            uint8_t value_size;
            float value[16];
        };
    
    public:
        
};