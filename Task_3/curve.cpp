#include <windows.h>
#include <vector>
#include <cmath>

using namespace std;

const vector<vector<float>> H = { {1, 0, 0, 0},
                {0, 1, 0, 0},
                {-3, -2, 3, -1},
                {2, 1, -2, 1}};
const int n = 1000;

struct Point {
    float x = 0.0, y = 0.0;
};

vector<vector<float>> multiply(const vector<vector<float>> &m1, const vector<vector<float>> &m2) {
    int r1 = m1.size();
    int c1 = m1[0].size();
    int c2 = m2[0].size();

    vector<vector<float>> C(r1, vector<float>(c2, 0));

    for (int i = 0; i < r1; i++)
        for (int j = 0; j < c2; j++)
            for (int k = 0; k < c1; k++)
                C[i][j] += m1[i][k] * m2[k][j];

    return C;
}

void hermite(HDC hdc, Point p1, Point T1, Point p2, Point T2) {
    vector<vector<float>> G = {{p1.x, p1.y},
             {T1.x, T1.y}, // u1, v1
             {p2.x, p2.y},
             {T2.x, T2.y}}; // u2, v2

    vector<vector<float>> C = multiply(H, G);

    for(float t = 0; t < 1; t += 1.0f/n) {
        vector<vector<float>> VT = {{1, t, t*t, t*t*t}};
        vector<vector<float>> X = multiply(VT, C);
        SetPixel(hdc, round(X[0][0]), round(X[0][1]), RGB(255,0,0));
    }

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void pointPaint(HDC hdc, int x, int y, COLORREF color)
{
    SetPixel(hdc, x, y, color);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static Point points[4];
    static Point points2[4];
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
            // P1, T1
            points2[0] = points[0]; // P1
            points2[1].x = points[1].x - points[0].x;
            points2[1].y = points[1].y - points[0].y;
            // P2, T2
            points2[2] = points[3]; // P2
            points2[3].x = points[3].x - points[2].x;
            points2[3].y = points[3].y - points[2].y;
            hermite(hdc, points2[0], points2[1], points2[2], points2[3]);
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