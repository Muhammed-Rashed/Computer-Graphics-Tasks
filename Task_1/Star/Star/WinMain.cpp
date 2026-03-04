#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <cmath>
#include <algorithm>

struct Point {
    int x, y;
    double angleWithCenter;
};

void calculateCenterAngles(Point points[], int size) {
    Point center;
    center.x = (points[0].x + points[1].x + points[2].x + points[3].x + points[4].x) / 5;
    center.y = (points[0].y + points[1].y + points[2].y + points[3].y + points[4].y) / 5;

    for(int i = 0; i < size; i++) {
        points[i].angleWithCenter = atan2(points[i].y - center.y, points[i].x - center.x);
    }
}

bool sortByAngle(Point p1, Point p2) { return p1.angleWithCenter < p2.angleWithCenter; }

void swap(int& x1, int& x2, int& y1, int& y2)
{
    int temp = x1;
    int temp2 = y1;
    x1 = x2;
    y1 = y2;
    x2 = temp;
    y2 = temp2;
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool pointClicked = false;
bool firstClick = true;
int x_start, y_start, x_end, y_end;
const COLORREF rgbRed = 0x000000FF;

void pointPaint(HDC hdc, int x, int y, COLORREF color) {
    SetPixel(hdc, x, y, color);
}
void linePaint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    if(abs(dy) <= abs(dx))
    {
        double m = (double)dy/dx;
        if(x1 > x2) swap(x1, x2, y1, y2);
        SetPixel(hdc, x1, y1, color);
        int x = x1;
        double y = y1;
        while(x < x2)
        {
            x++;
            y += m;
            SetPixel(hdc, x, round(y), color);
        }
    
    }
    else 
    {
        double mi = (double)dx/dy;
        if (y1 > y2) swap (x1, x2, y1, y2);
        SetPixel(hdc, x1, y1, color);
        int y = y1;
        double x = x1;
        while(y < y2)
        {
            y++;
            x += mi;
            SetPixel(hdc, round(x), y, color);
        }
    }
}

void drawStar(HDC hdc, Point points[]) {
    COLORREF colors[5] = {rgbRed, RGB(0, 255, 0), RGB(0, 0, 255), RGB(0, 255, 255), RGB(255, 0, 255)};
    for(int i = 0; i < 5; i++) {
        COLORREF color = colors[rand() % 5];
        linePaint(hdc, points[i].x, points[i].y, points[(i+2)%5].x, points[(i+2)%5].y, color);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Learn to Program Windows",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Point points[5];
    static int clicks = 0;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        points[clicks].x = LOWORD(lParam);
        points[clicks].y = HIWORD(lParam);
        clicks++;

        if(clicks >= 5)
        {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (clicks >= 5)
        {
            calculateCenterAngles(points, 5);
            std::sort(points, points + 5, sortByAngle);
            drawStar(hdc, points);
            clicks = 0;
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}