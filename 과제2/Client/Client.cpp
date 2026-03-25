#define WIN32_LEAN_AND_MEAN  // 충돌 방지를 위한 매크로 정의
#include <WS2tcpip.h>        // [cite: 498, 537] Winsock2 헤더를 먼저 포함
#include <windows.h>         // 그 다음에 windows.h 포함
#include <tchar.h>
#include <iostream>
#include <thread>
#include "protocol.h"

#pragma comment (lib, "WS2_32.LIB") // [cite: 500, 539]

// 전역 변수
SOCKET g_socket;
int g_pieceX = 0, g_pieceY = 0;
const int CELL_SIZE = 60;

// [cite: 42, 228] 서버로부터 데이터를 수신할 별도 스레드
void RecvThread(HWND hWnd) {
    while (true) {
        SC_POS_PACKET pos_pkt;
        // [cite: 311, 386] 서버의 좌표 패킷 대기
        int retval = recv(g_socket, (char*)&pos_pkt, sizeof(pos_pkt), MSG_WAITALL);
        if (retval <= 0) break;

        if (pos_pkt.type == PKT_SC_POS) {
            g_pieceX = pos_pkt.x;
            g_pieceY = pos_pkt.y;
            InvalidateRect(hWnd, NULL, TRUE); // [cite: 626] 화면 갱신
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN: {
        // [Debug] 클라이언트 콘솔에 키 입력 정보 출력 [cite: 531, 625]
        std::cout << "[Client] Key Pressed: " << wParam;
        if (wParam == VK_UP) std::cout << " (UP)";
        else if (wParam == VK_DOWN) std::cout << " (DOWN)";
        else if (wParam == VK_LEFT) std::cout << " (LEFT)";
        else if (wParam == VK_RIGHT) std::cout << " (RIGHT)";
        std::cout << std::endl;

        // 키 입력을 받아서 서버에 전송 [cite: 625]
        CS_MOVE_PACKET move_pkt;
        move_pkt.size = sizeof(move_pkt);
        move_pkt.type = PKT_CS_MOVE;
        move_pkt.vk_key = (int)wParam;
        send(g_socket, (char*)&move_pkt, sizeof(move_pkt), 0); // [cite: 400]
    } break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // 체스판 그리기
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                HBRUSH hBrush = CreateSolidBrush(((x + y) % 2 == 0) ? RGB(235, 235, 208) : RGB(119, 149, 86));
                RECT rect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
            }
        }
        // [cite: 626] 서버에서 받은 좌표로 말 그리기
        HBRUSH hPieceBrush = CreateSolidBrush(RGB(255, 0, 0));
        SelectObject(hdc, hPieceBrush);
        Ellipse(hdc, g_pieceX * CELL_SIZE + 10, g_pieceY * CELL_SIZE + 10, (g_pieceX + 1) * CELL_SIZE - 10, (g_pieceY + 1) * CELL_SIZE - 10);
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

// 윈도우 생성 함수 (WinMain 대용)
int RunWindow(HINSTANCE hInstance) {
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = _T("ChessClient");
    RegisterClassEx(&wcex);

    HWND hWnd = CreateWindow(_T("ChessClient"), _T("Chess Client"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 530, NULL, NULL, hInstance, NULL);
    ShowWindow(hWnd, SW_SHOW);

    // 수신 스레드 시작
    std::thread t(RecvThread, hWnd);
    t.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

int main() {
    // [cite: 627] 1. IP 주소 입력받기
    char serverIP[256];
    std::cout << "Server IP: ";
    std::cin >> serverIP;

    // 2. 네트워크 초기화 및 연결 [cite: 308, 515]
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    g_socket = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4000);
    inet_pton(AF_INET, serverIP, &addr.sin_addr); // [cite: 381, 514]

    if (connect(g_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Connect Failed!" << std::endl;
        return 0;
    }

    // 3. GUI 시작
    RunWindow(GetModuleHandle(NULL));
    return 0;
}