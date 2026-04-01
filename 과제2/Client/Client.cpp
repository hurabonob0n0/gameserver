#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include "../protocol.h"

#pragma comment (lib, "WS2_32.LIB")

SOCKET g_socket;
int g_pieceX = 0, g_pieceY = 0;
const int CELL_SIZE = 60;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN: {
        CS_MOVE_PACKET move_pkt;
        move_pkt.size = sizeof(move_pkt);
        move_pkt.type = PKT_CS_MOVE;
        move_pkt.vk_key = (int)wParam;

        int send_result = send(g_socket, (char*)&move_pkt, sizeof(move_pkt), 0);
        if (send_result == SOCKET_ERROR) break;

        std::cout << "[Client] Sent Key: " << wParam << std::endl;

        SC_POS_PACKET pos_pkt;
        int retval = recv(g_socket, (char*)&pos_pkt, sizeof(pos_pkt), MSG_WAITALL);

        if (retval > 0 && pos_pkt.type == PKT_SC_POS) {
            g_pieceX = pos_pkt.x;
            g_pieceY = pos_pkt.y;

            std::cout << "[Client] Received Pos: (" << g_pieceX << ", " << g_pieceY << ")" << std::endl;
            InvalidateRect(hWnd, NULL, TRUE);
        }
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                HBRUSH hBrush = CreateSolidBrush(((x + y) % 2 == 0) ? RGB(235, 235, 208) : RGB(119, 149, 86));
                RECT rect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
            }
        }
        HBRUSH hPieceBrush = CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(hdc, hPieceBrush);
        Ellipse(hdc, g_pieceX * CELL_SIZE + 5, g_pieceY * CELL_SIZE + 5, (g_pieceX + 1) * CELL_SIZE - 5, (g_pieceY + 1) * CELL_SIZE - 5);
        DeleteObject(hPieceBrush);
        EndPaint(hWnd, &ps);
    } break;

    case WM_DESTROY:
        closesocket(g_socket);
        WSACleanup();
        PostQuitMessage(0);
        break;
    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void RunWindow(HINSTANCE hInstance) {
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = _T("ChessClient");
    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindow(_T("ChessClient"), _T("Chess Client"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 530, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int main() {
    char serverIP[256];
    std::cout << "Server IP: ";
    std::cin >> serverIP;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    g_socket = socket(AF_INET, SOCK_STREAM, 0); 

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    inet_pton(AF_INET, serverIP, &addr.sin_addr);

    if (connect(g_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Connect Failed!" << std::endl;
        return 0;
    }

    std::cout << "Connected to Server. Move with Arrow Keys." << std::endl;

    RunWindow(GetModuleHandle(NULL));
    return 0;
}