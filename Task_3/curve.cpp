#include <windows.h>

struct Point
{
    int x, y;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void pointPaint(HDC hdc, int x, int y, COLORREF color)
{
    SetPixel(hdc, x, y, color);
}

void draw_hermit(HDC hdc, Point p[])
{
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Point points[4];
    static int clicks = 0;

    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        if (clicks < 4)
        {
            points[clicks].x = LOWORD(lParam);
            points[clicks].y = HIWORD(lParam);
            clicks++;
        }

        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        for (int i = 0; i < clicks; i++)
        {
            SetPixel(hdc, points[i].x, points[i].y, RGB(0, 0, 0));
        }

        if (clicks == 4)
        {
            draw_hermit(hdc, points);
            clicks = 0;
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int APIENTRY WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int nsh)
{
	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hInstance = h;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "Hermite Window";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);
	HWND hwnd = CreateWindow("Hermite Window", "Hermite Curve Input", 
		WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, h, 0);
	ShowWindow(hwnd, nsh);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;


}