#pragma once

// [cite: 191] 구조체 정렬을 1바이트로 설정하여 데이터 왜곡 방지
#pragma pack(push, 1) 

// [cite: 147, 160] 클라이언트 -> 서버: 키 입력 전송용 구조체
struct CS_MOVE_PACKET {
    unsigned char size; // [cite: 127] 패킷 크기
    unsigned char type; // [cite: 127] 패킷 종류 (0: 이동 요청)
    int vk_key;         // 가상 키 값 (VK_UP, VK_DOWN 등)
};

// [cite: 147, 160] 서버 -> 클라이언트: 좌표 전송용 구조체
struct SC_POS_PACKET {
    unsigned char size; // [cite: 127] 패킷 크기
    unsigned char type; // [cite: 127] 패킷 종류 (1: 위치 정보)
    int x;              // 서버에서 계산된 x 좌표
    int y;              // 서버에서 계산된 y 좌표
};

#pragma pack(pop)

// [cite: 145] 프로토콜 타입 정의
#define PKT_CS_MOVE 0
#define PKT_SC_POS  1