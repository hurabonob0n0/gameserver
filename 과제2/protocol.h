#pragma once
#pragma pack(push, 1) 

struct CS_MOVE_PACKET {
    unsigned char size;
    unsigned char type;
    int vk_key;        
};

struct SC_POS_PACKET {
    unsigned char size;
    unsigned char type;
    int x;             
    int y;             
};

#pragma pack(pop)

#define PKT_CS_MOVE 0
#define PKT_SC_POS  1