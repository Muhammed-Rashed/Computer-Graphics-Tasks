#include <windows.h>
#include <cmath>
#include <ctime>
using namespace std;

#define IDM_ELLIPSE_DIRECT 1
#define IDM_ELLIPSE_POLAR 2
#define IDM_ELLIPSE_MIDPOINT 3

enum Algo
{
    ELLIPSE_DIRECT,
    ELLIPSE_POLAR,
    ELLIPSE_MIDPOINT
};

void midpointEllipse(HDC hdc, int rx, int ry, int xc, int yc, COLORREF color)
{
    float dx, dy, d1, d2, x, y;
    x = 0;
    y = ry;

    d1 = (ry * ry) - (rx * rx * ry) + (0.25 * rx * rx);
    dx = 2 * ry * ry * x;
    dy = 2 * rx * rx * y;

    // Region 1
    while (dx < dy)
    {
        SetPixel(hdc, x + xc, y + yc, color);
        SetPixel(hdc, -x + xc, y + yc, color);
        SetPixel(hdc, x + xc, -y + yc, color);
        SetPixel(hdc, -x + xc, -y + yc, color);

        if (d1 < 0)
        {
            x++;
            dx = dx + (2 * ry * ry);
            d1 = d1 + dx + (ry * ry);
        }
        else
        {
            x++;
            y--;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d1 = d1 + dx - dy + (ry * ry);
        }
    }

    d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) + ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

    // Region 2
    while (y >= 0)
    {
        SetPixel(hdc, x + xc, y + yc, color);
        SetPixel(hdc, -x + xc, y + yc, color);
        SetPixel(hdc, x + xc, -y + yc, color);
        SetPixel(hdc, -x + xc, -y + yc, color);

        if (d2 > 0)
        {
            y--;
            dy = dy - (2 * rx * rx);
            d2 = d2 + (rx * rx) - dy;
        }
        else
        {
            y--;
            x++;
            dx = dx + (2 * ry * ry);
            dy = dy - (2 * rx * rx);
            d2 = d2 + dx - dy + (rx * rx);
        }
    }
}

// ((x - yc)^2 / rx^2) + ((y - xc)^2 / ry^2) = 1
void directEllipse(HDC hdc, int rx, int ry, int xc, int yc, COLORREF color)
{
    double y;

    for (int x = -rx; x <= rx; x++)
    {
        y = ry * sqrt(1.0 - ((double)(x * x) / (rx * rx)));

        SetPixel(hdc, xc + x, yc + y, color);
        SetPixel(hdc, xc + x, yc - y, color);
    }
}

LRESULT WINAPI WndProc(HWND hwnd, UINT mcode, WPARAM wp, LPARAM lp)
{
    static int clickCount = 0;
    static POINT points[3];

    static Algo currentAlgo = ELLIPSE_MIDPOINT;

    switch (mcode)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        points[clickCount].x = LOWORD(lp);
        points[clickCount].y = HIWORD(lp);

        clickCount++;

        // After 3 clicks draw ellipse
        if (clickCount == 3)
        {
            HDC hdc = GetDC(hwnd);

            int xc = points[0].x;
            int yc = points[0].y;

            int rx = abs(points[1].x - xc);
            int ry = abs(points[2].y - yc);

            switch (currentAlgo)
            {
            case ELLIPSE_DIRECT:
                directEllipse(hdc, rx, ry, xc, yc, RGB(255, 0, 0));
                break;

            //case ELLIPSE_POLAR:
                //polarEllipse(hdc, rx, ry, xc, yc, RGB(0, 255, 0));
                //break;

            case ELLIPSE_MIDPOINT:
                midpointEllipse(hdc, rx, ry, xc, yc, RGB(0, 0, 255));
                break;
            }

            ReleaseDC(hwnd, hdc);

            clickCount = 0;
        }

        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wp))
        {
        case IDM_ELLIPSE_DIRECT:
            currentAlgo = ELLIPSE_DIRECT;
            break;

        case IDM_ELLIPSE_POLAR:
            currentAlgo = ELLIPSE_POLAR;
            break;

        case IDM_ELLIPSE_MIDPOINT:
            currentAlgo = ELLIPSE_MIDPOINT;
            break;
        }

        return 0;
    }
    }

    return DefWindowProc(hwnd, mcode, wp, lp);
}

int APIENTRY WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int nsh)
{
    srand((unsigned)time(NULL));
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

    // Create menu bar
    HMENU hMenu = CreateMenu();

    // Create submenu
    HMENU hEllipseMenu = CreatePopupMenu();

    // Add submenu to menu bar
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEllipseMenu, "Ellipse");

    // Attach menu to window
    SetMenu(hwnd, hMenu);

    // Add items to submenu
    // -- Ellipse --
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_DIRECT, "Direct");
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_MIDPOINT, "Midpoint");
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_POLAR, "Polar");

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