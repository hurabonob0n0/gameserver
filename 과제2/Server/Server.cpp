#include <iostream>
#include <WS2tcpip.h>
#include "protocol.h"

// [cite: 500, 539] 윈속 라이브러리 링크
#pragma comment (lib, "WS2_32.LIB")

int main() {
    // [cite: 546] 윈속 초기화
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);

    // [cite: 341, 547] 리스닝 소켓 생성
    SOCKET l_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

    // [cite: 379, 550] 서버 주소 설정
    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4000); // 포트 4000번
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // [cite: 464, 552]

    // [cite: 460, 553] 소켓 바인드 및 리슨
    bind(l_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(l_socket, SOMAXCONN); // [cite: 476, 554]

    std::cout << "Chess Server Waiting..." << std::endl;

    // [cite: 480, 556] 클라이언트 접속 수용
    int addr_size = sizeof(server_addr);
    SOCKET c_socket = WSAAccept(l_socket, (sockaddr*)&server_addr, &addr_size, 0, 0);

    //  게임 상태 관리 (서버에서 관리)
    int g_pieceX = 0, g_pieceY = 0;

    while (true) {
        CS_MOVE_PACKET move_pkt;
        // 데이터 수신 [cite: 386, 563]
        int retval = recv(c_socket, (char*)&move_pkt, sizeof(move_pkt), MSG_WAITALL);
        if (retval <= 0) break;

        // [Debug] 수신 정보 및 이전 좌표 출력 [cite: 565, 629]
        std::cout << "[Server] Received Key: " << move_pkt.vk_key;
        std::cout << " | Current Pos: (" << g_pieceX << ", " << g_pieceY << ")";

        //  로직 처리: 서버에서 말의 위치 결정 [cite: 629]
        switch (move_pkt.vk_key) {
        case VK_UP:    if (g_pieceY > 0) g_pieceY--; break;
        case VK_DOWN:  if (g_pieceY < 7) g_pieceY++; break;
        case VK_LEFT:  if (g_pieceX > 0) g_pieceX--; break;
        case VK_RIGHT: if (g_pieceX < 7) g_pieceX++; break;
        }

        // [Debug] 이동 후의 좌표 출력 
        std::cout << " -> New Pos: (" << g_pieceX << ", " << g_pieceY << ")" << std::endl;

        // 변경된 위치를 클라이언트에 전송 [cite: 400, 568]
        SC_POS_PACKET pos_pkt;
        pos_pkt.size = sizeof(pos_pkt);
        pos_pkt.type = PKT_SC_POS;
        pos_pkt.x = g_pieceX;
        pos_pkt.y = g_pieceY;
        send(c_socket, (char*)&pos_pkt, sizeof(pos_pkt), 0);
    }

    closesocket(c_socket); // [cite: 454]
    closesocket(l_socket);
    WSACleanup(); // [cite: 570]
    return 0;
}