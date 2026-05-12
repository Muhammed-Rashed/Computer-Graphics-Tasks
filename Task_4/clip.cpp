#include <windows.h>
#include <cmath>

using namespace std;

#define IDM_RECT_POINT      1
#define IDM_RECT_LINE       2
#define IDM_SQUARE_POINT    3
#define IDM_SQUARE_LINE     4
#define IDM_CIRCLE_POINT    5
#define IDM_CIRCLE_LINE     6


// Clipping Modes
enum ClipMode
{
    NONE,
    RECT_POINT,
    RECT_LINE,
    SQUARE_POINT,
    SQUARE_LINE,
    CIRCLE_POINT,
    CIRCLE_LINE
};


// Global Variables
ClipMode currentMode = NONE;
POINT points[4];
int clickCount = 0;


//  Point Clipping
void PointClipping(HDC hdc, int x, int y, int xleft, int ytop, int xright, int ybottom, COLORREF color)
{
    if (x >= xleft && x <= xright && y >= ytop && y <= ybottom)
    {
        SetPixel(hdc, x, y, color);
    }
}


// Line Clipping
union OutCode
{
    unsigned All : 4;
    struct {unsigned left : 1, top : 1, right : 1, bottom : 1;};
};

OutCode GetOutCode(double x, double y, int xleft, int ytop, int xright, int ybottom)
{
    OutCode out;
    out.All = 0;
    if (x < xleft) out.left = 1; else if (x > xright) out.right = 1;
    if (y < ytop) out.top = 1; else if (y > ybottom) out.bottom = 1;
    return out;
}

void VIntersect(double xs, double ys, double xe, double ye, int x, double* xi, double* yi)
{
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}

void HIntersect(double xs, double ys, double xe, double ye, int y, double* xi, double* yi)
{
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

void CohenSutherlandClip(HDC hdc, int xs, int ys, int xe, int ye, int xleft, int ytop, int xright, int ybottom, COLORREF color)
{
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    OutCode out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
    OutCode out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);

    while ((out1.All || out2.All) && !(out1.All & out2.All))
    {
        double xi, yi;
        if (out1.All)
        {
            if (out1.left) VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out1.top) HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out1.right) VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            x1 = xi;
            y1 = yi;
            out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
        }
        else
        {
            if (out2.left) VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out2.top) HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out2.right) VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            x2 = xi;
            y2 = yi;
            out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);
        }
    }

    if (!out1.All && !out2.All)
    {
        MoveToEx(hdc, round(x1), round(y1), NULL);
        LineTo(hdc, round(x2), round(y2));
    }
}


// Circle Window Clipping
bool insideCircle(int x, int y, int xc, int yc, int r)
{
    int dx = x - xc;
    int dy = y - yc;
    return (dx * dx + dy * dy) <= r * r;
}

void circlePointClipping(HDC hdc, int x, int y, int xc, int yc, int r, COLORREF color)
{
    if(insideCircle(x,y,xc,yc,r))
    {
        SetPixel(hdc,x,y,color);
    }
}

void circleLineClipping(HDC hdc, int x1, int y1, int x2, int y2, int xc, int yc, int r, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    double xInc = (double)dx / steps;
    double yInc = (double)dy / steps;

    double x = x1;
    double y = y1;

    for(int i = 0; i <= steps; i++)
    {
        if(insideCircle(round(x), round(y), xc,yc,r))
        {
            SetPixel(hdc, round(x), round(y), color);
        }

        x += xInc;
        y += yInc;
    }
}


LRESULT WINAPI WndProc(HWND hwnd, UINT mcode, WPARAM wp, LPARAM lp)
{
    switch(mcode)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        // Mouse
        case WM_LBUTTONDOWN:
        {
            HDC hdc = GetDC(hwnd);

            points[clickCount].x = LOWORD(lp);
            points[clickCount].y = HIWORD(lp);

            clickCount++;

            // Rectangle Point
            if(currentMode == RECT_POINT)
            {
                if(clickCount == 2)
                {
                    Rectangle(hdc, points[0].x, points[0].y, points[1].x, points[1].y);
                }

                else if(clickCount == 3)
                {
                    int xleft = min(points[0].x, points[1].x);
                    int xright = max(points[0].x, points[1].x);
                    int ytop = min(points[0].y, points[1].y);
                    int ybottom = max(points[0].y, points[1].y);

                    PointClipping(hdc, points[2].x, points[2].y, xleft, ytop, xright, ybottom, RGB(255,0,0));
                    clickCount = 0;
                }
            }

            
            // Rectangle Line
            else if(currentMode == RECT_LINE)
            {
                if(clickCount == 2)
                {
                    Rectangle(hdc, points[0].x, points[0].y, points[1].x, points[1].y);
                }

                else if(clickCount == 4)
                {
                    int xleft = min(points[0].x, points[1].x);
                    int xright = max(points[0].x, points[1].x);
                    int ytop = min(points[0].y, points[1].y);
                    int ybottom = max(points[0].y, points[1].y);

                    CohenSutherlandClip(hdc, points[2].x, points[2].y, points[3].x, points[3].y, xleft, ytop, xright, ybottom, RGB(0,0,255));
                    clickCount = 0;
                }
            }

            
            // Square Point
            else if(currentMode == SQUARE_POINT)
            {
                if(clickCount == 2)
                {
                    int side = min(abs(points[1].x - points[0].x), abs(points[1].y - points[0].y));
                    Rectangle(hdc, points[0].x, points[0].y, points[0].x + side, points[0].y + side);
                }

                else if(clickCount == 3)
                {
                    int side = min(abs(points[1].x - points[0].x), abs(points[1].y - points[0].y));
                    PointClipping(hdc, points[2].x, points[2].y, points[0].x, points[0].y, points[0].x + side, points[0].y + side, RGB(255,0,255));
                    clickCount = 0;
                }
            }

            
            // Square Line
            else if(currentMode == SQUARE_LINE)
            {
                if(clickCount == 2)
                {
                    int side = min(abs(points[1].x - points[0].x), abs(points[1].y - points[0].y));
                    Rectangle(hdc, points[0].x, points[0].y, points[0].x + side, points[0].y + side);
                }

                else if(clickCount == 4)
                {
                    int side = min(abs(points[1].x - points[0].x), abs(points[1].y - points[0].y));
                    CohenSutherlandClip(hdc, points[2].x, points[2].y, points[3].x, points[3].y, points[0].x, points[0].y, points[0].x + side, points[0].y + side, RGB(0,255,0));
                    clickCount = 0;
                }
            }

            
            // Circle Point
            else if(currentMode == CIRCLE_POINT)
            {
                if(clickCount == 2)
                {
                    int xc = points[0].x;
                    int yc = points[0].y;

                    int r = sqrt(pow(points[1].x - xc,2) + pow(points[1].y - yc,2));

                    Ellipse(hdc, xc - r, yc - r, xc + r, yc + r);
                }

                else if(clickCount == 3)
                {
                    int xc = points[0].x;
                    int yc = points[0].y;

                    int r = sqrt(pow(points[1].x - xc,2) + pow(points[1].y - yc,2));

                    circlePointClipping(hdc, points[2].x, points[2].y, xc, yc, r, RGB(255,128,0));
                    clickCount = 0;
                }
            }

        
            // Circle Line
            else if(currentMode == CIRCLE_LINE)
            {
                if(clickCount == 2)
                {
                    int xc = points[0].x;
                    int yc = points[0].y;

                    int r = sqrt(pow(points[1].x - xc,2) + pow(points[1].y - yc,2));

                    Ellipse(hdc, xc - r, yc - r, xc + r, yc + r);
                }

                else if(clickCount == 4)
                {
                    int xc = points[0].x;
                    int yc = points[0].y;

                    int r = sqrt(pow(points[1].x - xc,2) + pow(points[1].y - yc,2));

                    circleLineClipping(hdc, points[2].x, points[2].y, points[3].x, points[3].y, xc, yc, r, RGB(0,128,255));
                    clickCount = 0;
                }
            }
            ReleaseDC(hwnd,hdc);
            return 0;
        }

        // Menu
        case WM_COMMAND:
        {
            switch(LOWORD(wp))
            {
            case IDM_RECT_POINT:
                currentMode = RECT_POINT;
                clickCount = 0;
                break;

            case IDM_RECT_LINE:
                currentMode = RECT_LINE;
                clickCount = 0;
                break;

            case IDM_SQUARE_POINT:
                currentMode = SQUARE_POINT;
                clickCount = 0;
                break;

            case IDM_SQUARE_LINE:
                currentMode = SQUARE_LINE;
                clickCount = 0;
                break;

            case IDM_CIRCLE_POINT:
                currentMode = CIRCLE_POINT;
                clickCount = 0;
                break;

            case IDM_CIRCLE_LINE:
                currentMode = CIRCLE_LINE;
                clickCount = 0;
                break;
            }
            return 0;
        }
    }

    return DefWindowProc(hwnd, mcode, wp, lp);
}


// WinMain
int APIENTRY WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int nsh)
{
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = h;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "myclass";
    wc.lpszMenuName = NULL;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&wc);
    HWND hwnd = CreateWindow("myclass", "Clipping",
                             WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, h, 0);

    // Menus
    HMENU hMenu = CreateMenu();
    HMENU hClipMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hClipMenu, "Clipping Menu");
    AppendMenu(hClipMenu, MF_STRING, IDM_RECT_POINT, "Rectangle Point");
    AppendMenu(hClipMenu, MF_STRING, IDM_RECT_LINE, "Rectangle Line");
    AppendMenu(hClipMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hClipMenu, MF_STRING, IDM_SQUARE_POINT, "Square Point");
    AppendMenu(hClipMenu, MF_STRING, IDM_SQUARE_LINE, "Square Line");
    AppendMenu(hClipMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hClipMenu, MF_STRING, IDM_CIRCLE_POINT, "Circle Point");
    AppendMenu(hClipMenu, MF_STRING, IDM_CIRCLE_LINE, "Circle Line");
    SetMenu(hwnd,hMenu);

    // Show Window
    ShowWindow(hwnd,nsh);
    UpdateWindow(hwnd);
    MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}