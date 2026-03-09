#include <windows.h>
#include <tchar.h>

// 전역 변수
const int BOARD_SIZE = 8;
const int CELL_SIZE = 60; // 한 칸의 크기 (픽셀)
int g_pieceX = 0;         // 말의 X 좌표 (0~7)
int g_pieceY = 0;         // 말의 Y 좌표 (0~7)

// 윈도우 프로시저 선언
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = _T("ChessClientTest");

    if (!RegisterClassEx(&wcex)) return 0;

    // 화면 중앙에 적절한 크기로 윈도우 생성
    HWND hWnd = CreateWindow(_T("ChessClientTest"), _T("Chess Client - Move with Arrow Keys"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CELL_SIZE * BOARD_SIZE + 20, CELL_SIZE * BOARD_SIZE + 45,
        NULL, NULL, hInstance, NULL);

    if (!hWnd) return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN: // 키보드 입력 처리
        switch (wParam) {
        case VK_UP:    if (g_pieceY > 0) g_pieceY--; break;
        case VK_DOWN:  if (g_pieceY < BOARD_SIZE - 1) g_pieceY++; break;
        case VK_LEFT:  if (g_pieceX > 0) g_pieceX--; break;
        case VK_RIGHT: if (g_pieceX < BOARD_SIZE - 1) g_pieceX++; break;
        }
        InvalidateRect(hWnd, NULL, TRUE); // 화면 갱신 요청
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 1. 체스 판 그리기 (8x8)
        for (int y = 0; y < BOARD_SIZE; ++y) {
            for (int x = 0; x < BOARD_SIZE; ++x) {
                HBRUSH hBrush = CreateSolidBrush(((x + y) % 2 == 0) ? RGB(235, 235, 208) : RGB(119, 149, 86));
                RECT rect = { x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
            }
        }

        // 2. 체스 말 그리기 (단순 원형)
        HBRUSH hPieceBrush = CreateSolidBrush(RGB(255, 0, 0)); // 빨간색 말
        SelectObject(hdc, hPieceBrush);
        int margin = 10;
        Ellipse(hdc,
            g_pieceX * CELL_SIZE + margin,
            g_pieceY * CELL_SIZE + margin,
            (g_pieceX + 1) * CELL_SIZE - margin,
            (g_pieceY + 1) * CELL_SIZE - margin);
        DeleteObject(hPieceBrush);

        EndPaint(hWnd, &ps);
    } break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}