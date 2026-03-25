#include <iostream>
#include <WS2tcpip.h>
#include "../protocol.h"

#pragma comment (lib, "WS2_32.LIB")

int main() {
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);

    SOCKET l_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

    SOCKADDR_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(7777); 
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    bind(l_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(l_socket, SOMAXCONN);

    std::cout << "Chess Server Waiting..." << std::endl;


    int addr_size = sizeof(server_addr);
    SOCKET c_socket = WSAAccept(l_socket, (sockaddr*)&server_addr, &addr_size, 0, 0);


    int g_pieceX = 0, g_pieceY = 0;

    while (true) {
        CS_MOVE_PACKET move_pkt;
        int retval = recv(c_socket, (char*)&move_pkt, sizeof(move_pkt), MSG_WAITALL);
        if (retval <= 0) break;

        std::cout << "[Server] Received Key: " << move_pkt.vk_key;
        std::cout << " | Current Pos: (" << g_pieceX << ", " << g_pieceY << ")";

        switch (move_pkt.vk_key) {
        case VK_UP:    if (g_pieceY > 0) g_pieceY--; break;
        case VK_DOWN:  if (g_pieceY < 7) g_pieceY++; break;
        case VK_LEFT:  if (g_pieceX > 0) g_pieceX--; break;
        case VK_RIGHT: if (g_pieceX < 7) g_pieceX++; break;
        }

        std::cout << " -> New Pos: (" << g_pieceX << ", " << g_pieceY << ")" << std::endl;

        SC_POS_PACKET pos_pkt;
        pos_pkt.size = sizeof(pos_pkt);
        pos_pkt.type = PKT_SC_POS;
        pos_pkt.x = g_pieceX;
        pos_pkt.y = g_pieceY;
        send(c_socket, (char*)&pos_pkt, sizeof(pos_pkt), 0);
    }

    closesocket(c_socket);
    closesocket(l_socket);
    WSACleanup();
    return 0;
}