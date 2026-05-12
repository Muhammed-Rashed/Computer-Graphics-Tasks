#include <windows.h>
#include <cmath>
#include <fstream>
#include <ctime>
using namespace std;

#define IDM_ELLIPSE_DIRECT 1
#define IDM_ELLIPSE_POLAR 2
#define IDM_ELLIPSE_MIDPOINT 3

// helper functions

//clear function
void clear(HWND hwnd, COLORREF bgColor)
{
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    for(int x = 0; x < rect.right; x++)
    {
        for(int y = 0; y < rect.bottom; y++)
        {
            SetPixel(hdc, x, y, bgColor);
        }
    }
    ReleaseDC(hwnd, hdc);
}

// save function
void save(HWND hwnd, COLORREF bgColor)
{
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    ofstream file("shapes.txt");
    for(int x = 0; x < rect.right; x++)
    {
        for(int y = 0; y < rect.bottom; y++)
        {
            COLORREF color = GetPixel(hdc, x, y);
            if (color != bgColor) // if not background color
            {
                file << x << " " << y << " " << (int)color << endl;
            }
        }
    }
    file.close();
    ReleaseDC(hwnd, hdc);
}

// load function
void load(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    ifstream file("shapes.txt");
    int x, y;
    int color;
    while (file >> x >> y >> color)
    {
        SetPixel(hdc, x, y, (COLORREF)color);
    }
    file.close();
    ReleaseDC(hwnd, hdc);
}

void swap(int& x1, int& x2, int& y1, int& y2)
{
    int temp = x1;
    int temp2 = y1;
    x1 = x2;
    y1 = y2;
    x2 = temp;
    y2 = temp2;
}

void draw8points(HDC hdc, int xc, int yc, int x, int y, COLORREF color)
{
    SetPixel(hdc, xc + x, yc + y, color);
    SetPixel(hdc, xc - x, yc + y, color);
    SetPixel(hdc, xc + x, yc - y, color);
    SetPixel(hdc, xc - x, yc - y, color);
    SetPixel(hdc, xc + y, yc + x, color);
    SetPixel(hdc, xc - y, yc + x, color);
    SetPixel(hdc, xc + y, yc - x, color);
    SetPixel(hdc, xc - y, yc - x, color);
}

// line algorithms

void lineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
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

// Line midpoint
void lineMidPoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (abs(dy) <= abs(dx))
    {
        if (x1 > x2)
            swap(x1, x2, y1, y2);

        dx = x2 - x1;
        dy = y2 - y1;

        int d = 2 * abs(dy) - abs(dx);
        int change1 = 2 * (abs(dy) - abs(dx));
        int change2 = 2 * abs(dy);

        int x = x1;
        int y = y1;

        int yStep = (dy >= 0) ? 1 : -1;

        SetPixel(hdc, x, y, color);

        while (x < x2)
        {
            if (d > 0)
            {
                d += change1;
                y += yStep;
            }
            else
            {
                d += change2;
            }

            x++;
            SetPixel(hdc, x, y, color);
        }
    }
    else
    {
        if (y1 > y2)
            swap(x1, x2, y1, y2);

        dx = x2 - x1;
        dy = y2 - y1;

        int d = 2 * abs(dx) - abs(dy);
        int change1 = 2 * (abs(dx) - abs(dy));
        int change2 = 2 * abs(dx);

        int x = x1;
        int y = y1;

        int xStep = (dx >= 0) ? 1 : -1;

        SetPixel(hdc, x, y, color);

        while (y < y2)
        {
            if (d > 0)
            {
                d += change1;
                x += xStep;
            }
            else
            {
                d += change2;
            }

            y++;
            SetPixel(hdc, x, y, color);
        }
    }
}

// line parametric
void lineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    for(double t = 0; t <= 1; t += 1.0 / steps)
    {
        int x = round(x1 + t * dx);
        int y = round(y1 + t * dy);

        SetPixel(hdc, x, y, color);
    }
}

// Circle algorithms

// Direct circle algo
void circleDirect(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    int x = 0, y = r;
    int r2 = r * r;
    draw8points(hdc, xc, yc, x, y, color);
    while (x < y)
    {
        x++;
        y = round(sqrt((double)(r2 - x * x)));
        draw8points(hdc, xc, yc, x, y, color);
    }
}

// Polar circle algo
void circlePolar(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    int x = r, y = 0;
    double theta = 0, dtheta = 1.0 / r;
    draw8points(hdc, xc, yc, x, y, color);
    while(x > y)
    {
        theta += dtheta;
        x = round(r * cos(theta));
        y = round(r * sin(theta));
        draw8points(hdc, xc, yc, x, y, color);
    }
}

// iterative polar circle algo
void circleIterativePolar(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    double x = r, y = 0;
    double dtheta = 1.0 / r;
    double cdtheta = cos(dtheta);
    double sdtheta = sin(dtheta);
    draw8points(hdc, xc, yc, x, y, color);
    while (x > y)
    {
        double x1 = x * cdtheta - y * sdtheta;
        y = x * sdtheta + y * cdtheta;
        x = x1;
        draw8points(hdc, xc, yc, round(x), round(y), color);
    }
}

// Midpoint circle algo
void circleMidpoint(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    int x = 0, y = r;
    int d = 1 - r;
    draw8points(hdc, xc, yc, x, y, color);
    while (x < y)
    {
        if (d < 0)
        {
            d += 2 * x + 3;
        }
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        draw8points(hdc, xc, yc, x, y, color);
    }
}
// modified midpoint algo
void circleModifiedMidPoint(HDC hdc, int xc, int yc, int r, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw8points(hdc, xc, yc, x, y, color);
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
        draw8points(hdc, xc, yc, x, y, color);
    }
}

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

// ((x - xc)^2 / rx^2) + ((y - yc)^2 / ry^2) = 1
void directEllipse(HDC hdc, int rx, int ry, int xc, int yc, COLORREF color)
{
    float x, y;

    for (x = -rx; x <= rx; x++)
    {
        y = ry * sqrt(1.0 - ((x * x) / (rx * rx)));

        SetPixel(hdc, xc + x, yc + y, color);
        SetPixel(hdc, xc + x, yc - y, color);
    }
}

// x = xc + (rx . cos(@))
// y = yc + (ry . sin(@))
void polarEllipse(HDC hdc, int rx, int ry, int xc, int yc, COLORREF color){
    float x, y;
    float angle = 0;
    float step = 0.01;

    while(angle <= 2 * M_PI) {
        x = xc + (rx * cos(angle));
        y = yc + (ry * sin(angle));

        SetPixel(hdc, round(x), round(y), color);

        angle += step;
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

            case ELLIPSE_POLAR:
                polarEllipse(hdc, rx, ry, xc, yc, RGB(0, 255, 0));
                break;

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