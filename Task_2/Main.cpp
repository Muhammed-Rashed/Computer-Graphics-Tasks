#include <windows.h>
#include <cmath>
void draw7points(HDC hdc, int xc, int yc, int x, int y)
{
    SetPixel(hdc, xc + x, yc + y, RGB(0, 255, 0));
    SetPixel(hdc, xc - x, yc + y, RGB(0, 255, 0));
    SetPixel(hdc, xc + x, yc - y, RGB(0, 0, 255));
    SetPixel(hdc, xc - x, yc - y, RGB(255, 0, 0));
    SetPixel(hdc, xc - y, yc + x, RGB(128, 0, 128));
    SetPixel(hdc, xc + y, yc - x, RGB(139, 0, 0));
    SetPixel(hdc, xc - y, yc - x, RGB(0, 0, 0));
}
void drawCircle(HDC hdc, int xc, int yc, int r)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw7points(hdc, xc, yc, x, y);
    while (x < y)
    {
        if (d < 0)
        {
            d += change1;
            change2 += 2;
        }
        else
        {
            d += change2;
            change2 += 4;
            y--;
        }
        change1 += 2;
        x++;
        draw7points(hdc, xc, yc, x, y);
    }
}

void drawShape(HDC hdc, int xc, int yc, int R1, int R2)
{
    // Filling between the 2 circles
    for (int r = R2; r >= R1; r--)
    {
        drawCircle(hdc, xc, yc, r);
    }
}

struct Point {
    int x = 0, y = 0;
};

double calculateDist(Point p1, Point p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;

    return sqrt(dx*dx + dy*dy);
}

LRESULT WINAPI WndProc(HWND hwnd, UINT mcode, WPARAM wp, LPARAM lp)
{
	static Point points[3]; // points[0] -> center, points[1] -> first circle, points[2] second circle
    static int clicks = 0; // Number of clicks

    switch (mcode)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        points[clicks].x = LOWORD(lp);
        points[clicks].y = HIWORD(lp);
        
        clicks++;
        InvalidateRect(hwnd, NULL, FALSE);

        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        for(int i = 0; i < clicks; i++)
        {
            SetPixel(hdc, points[i].x, points[i].y, RGB(0,0,0));
        }

        if (clicks == 3)
        {
            int R1 = (int) calculateDist(points[0], points[1]);
            int R2 = (int) calculateDist(points[0], points[2]);
            if (R2 < R1) std::swap(R1, R2);

            drawShape(hdc, points[0].x, points[0].y, R1, R2);
            clicks = 0;
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hwnd, mcode, wp, lp);
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
	wc.lpszClassName = "myclass";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wc);
	HWND hwnd = CreateWindow("myclass", "Hello World", 
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