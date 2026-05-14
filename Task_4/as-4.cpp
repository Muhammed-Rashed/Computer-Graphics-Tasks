#include <windows.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <algorithm>
#include <ctime>

#include "headers/line.h"
#include "headers/ellipse.h"
#include "headers/circle.h"
#include "headers/clip.h"
#include "headers/curvefill.h"

using namespace std;

const double PI = 3.141592653589793;
const COLORREF BG_COLOR = RGB(255, 255, 255);
COLORREF drawColor = RGB(255, 0, 0);
// ---- Mode enum ----
enum AppMode
{
    MODE_NONE,

    // Clipping
    MODE_RECT_POINT,
    MODE_RECT_LINE,
    MODE_SQUARE_POINT,
    MODE_SQUARE_LINE,
    MODE_CIRCLE_POINT,
    MODE_CIRCLE_LINE,
    MODE_RECT_POLYGON,
    MODE_SQUARE_POLYGON,

    // Ellipse
    MODE_ELLIPSE_DIRECT,
    MODE_ELLIPSE_POLAR,
    MODE_ELLIPSE_MIDPOINT,

    // Curves
    MODE_HERMIT,   // 4 clicks: p1, T1_endpoint, p2, T2_endpoint
    MODE_CARDINAL, // N clicks, right-click to finish

    // Circle Draw
    MODE_CIRCLE_DIRECT,
    MODE_CIRCLE_POLAR,
    MODE_CIRCLE_POLAR_ITERATIVE,
    MODE_CIRCLE_MIDPOINT,
    MODE_CIRCLE_MIDPOINT_MODIFIED,

    // Circle Fill
    MODE_CIRCLE_QUARTER,
    MODE_CIRCLE_LINE_FILL,
    MODE_POLYGON_CONVEX,
    MODE_POLYGON_GENERAL,
    MODE_FLOODFILL_REC,
    MODE_FLOODFILL_ITR,
    MODE_FILL_SQUARE_HERMIT,
    MODE_FILL_RECT_BEZIER,

    // Line
    MODE_LINE_DDA,
    MODE_LINE_MIDPOINT,
    MODE_LINE_PARAMETRIC,

    // Custom
    MODE_SMILE,
    MODE_SAD
};

// bonus faces part

// happy face
void drawHappy(HDC hdc, int xc, int yc)
{
    // 1. Draw and Fill the Head
    circleMidpoint(hdc, xc, yc, 80, RGB(0, 0, 0)); // Black Border
    // fill with yellow
    COLORREF cb = GetPixel(hdc, xc, yc);
    floodfillIterative(hdc, xc, yc, cb, RGB(255, 255, 0));
    

    // 2. Draw the Nose (Triangle-ish)
    lineMidPoint(hdc, xc - 5, yc + 15, xc + 5, yc + 15, RGB(0, 0, 0));
    lineMidPoint(hdc, xc, yc, xc + 5, yc + 15, RGB(0, 0, 0));

    // 3. Draw the Eyes
    // Outer circles (white of the eyes or just borders)
    circleMidpoint(hdc, xc - 30, yc - 20, 15, RGB(0, 0, 0));
    circleMidpoint(hdc, xc + 30, yc - 20, 15, RGB(0, 0, 0));

    // Inner Pupils (Filled black)
    circleMidpoint(hdc, xc - 30, yc - 20, 6, RGB(0, 0, 0));
    // fill with black
    cb = GetPixel(hdc, xc - 30, yc - 20);
    floodfillIterative(hdc, xc - 30, yc - 20, cb, RGB(0, 0, 0));

    circleMidpoint(hdc, xc + 30, yc - 20, 6, RGB(0, 0, 0));
    // fill with black
    cb = GetPixel(hdc, xc + 30, yc - 20); 
    floodfillIterative(hdc, xc + 30, yc - 20, cb, RGB(0, 0, 0));

    // 4. THE SMILE (Hermite Curve)
    Point p1, p2, T1, T2;

    // Start point of smile (left side)
    p1.x = xc - 34;
    p1.y = yc + 30;

    // End point of smile (right side)
    p2.x = xc + 34;
    p2.y = yc + 30;

    // Tangent T1: Points DOWN and slightly right
    T1.x = 20;
    T1.y = 100; // High Y value creates the downward dip

    // Tangent T2: Points UP and slightly right
    T2.x = 20;
    T2.y = -100; // Negative Y value pulls the curve back up

    hermit(hdc, p1, T1, p2, T2, RGB(0, 0, 0));
}

// sad face
void drawSad(HDC hdc, int xc, int yc)
{
    // 1. Draw and Fill the Head
    circleMidpoint(hdc, xc, yc, 80, RGB(0, 0, 0)); // Black Border
    // fill with yellow
    COLORREF cb = GetPixel(hdc, xc, yc);
    floodfillIterative(hdc, xc, yc, cb, RGB(255, 255, 0));
    

    // 2. Draw the Nose (Triangle-ish)
    lineMidPoint(hdc, xc - 5, yc + 15, xc + 5, yc + 15, RGB(0, 0, 0));
    lineMidPoint(hdc, xc, yc, xc + 5, yc + 15, RGB(0, 0, 0));

    // 3. Draw the Eyes
    // Outer circles (white of the eyes or just borders)
    circleMidpoint(hdc, xc - 30, yc - 20, 15, RGB(0, 0, 0));
    circleMidpoint(hdc, xc + 30, yc - 20, 15, RGB(0, 0, 0));

    // Inner Pupils (Filled black)
    circleMidpoint(hdc, xc - 30, yc - 20, 6, RGB(0, 0, 0));
    // fill with black
    cb = GetPixel(hdc, xc - 30, yc - 20);
    floodfillIterative(hdc, xc - 30, yc - 20, cb, RGB(0, 0, 0));

    circleMidpoint(hdc, xc + 30, yc - 20, 6, RGB(0, 0, 0));
    // fill with black
    cb = GetPixel(hdc, xc + 30, yc - 20); 
    floodfillIterative(hdc, xc + 30, yc - 20, cb, RGB(0, 0, 0));

    // 4. THE FROWN (Inverted Hermite Curve)
    Point p1, p2, T1, T2;
    
    // Position the frown slightly lower than the smile
    p1.x = xc - 30; 
    p1.y = yc + 45; 
    
    p2.x = xc + 30;
    p2.y = yc + 45;
    
    // Inverting the tangents:
    // T1.y is now negative (points UP first)
    T1.x = 20;   
    T1.y = -70; 
    
    // T2.y is now positive (comes from UP to DOWN)
    T2.x = 20;
    T2.y = 70; 
    
    // Draw the frown in black
    hermit(hdc, p1, T1, p2, T2, RGB(0,0,0)); 
}

// file menu part

struct Shape
{
    int type;
    COLORREF color;
    vector<Point> pts;
};

vector<Shape> shapes;
// clear
void clear(HWND hwnd, COLORREF bgColor)
{
    HDC hdc = GetDC(hwnd);

    RECT rect;
    GetClientRect(hwnd, &rect);

    HBRUSH brush = CreateSolidBrush(bgColor);

    FillRect(hdc, &rect, brush);

    DeleteObject(brush);
    ReleaseDC(hwnd, hdc);
}

// save
void save()
{
    ofstream file("shapes.txt");

    for (auto &s : shapes)
    {
        file << s.type << ' '
             << s.color << ' '
             << s.pts.size() << '\n';

        for (auto &p : s.pts)
        {
            file << p.x << ' ' << p.y << '\n';
        }
    }

    file.close();
}

// load
void load(HWND hwnd)
{
    ifstream file("shapes.txt");
    if (!file.is_open())
        return; // Safety check

    shapes.clear();
    clear(hwnd, BG_COLOR); // Wipe the screen before reloading

    HDC hdc = GetDC(hwnd);
    Shape s;
    int sz;
    // This is the robust way to check for end-of-file while reading
    while (file >> s.type >> s.color >> sz)
    {
        s.pts.clear(); // Clear previous points from the shape object
        for (int i = 0; i < sz; i++)
        {
            Point p;
            if (!(file >> p.x >> p.y))
                break;
            s.pts.push_back(p);
        }

        shapes.push_back(s);

        // Immediate Drawing
        if (s.type == MODE_CIRCLE_MIDPOINT && s.pts.size() >= 2)
        {
            circleMidpoint(hdc, s.pts[0].x, s.pts[0].y, s.pts[1].x, s.color);
        }
        else if (s.type == MODE_HERMIT && s.pts.size() >= 4)
        {
            hermit(hdc, s.pts[0], s.pts[1], s.pts[2], s.pts[3], s.color);
        }
    }

    ReleaseDC(hwnd, hdc);
    file.close();
}

/*
HOW TO ADD NEW FEATURES
========================

1. Add a menu ID at the top:
#define IDM_SOMETHING 701
Use a unique number. Current ranges:
    Clipping=1xx, Ellipse=2xx, Curves=3xx,
    Circle Fill=4xx, Circle=5xx, Utility=6xx

2. If it needs mouse clicks on the canvas (drawing) then add a mode to the enum:
MODE_SOMETHING,
If it runs immediately on menu click (e.g. Clear, Save, change color) SKIP THIS STEP.

3. (Drawing modes only) Add input handling in WM_LBUTTONDOWN:
    else if (mode == MODE_SOMETHING)
    {
        if (clickCount == 2)   // adjust click count as needed
        {
            // draw using pts[0], pts[1], etc.
            clickCount = 0;
        }
    }

4. (Drawing modes only) Add a hint in UpdateTitle():
    case MODE_SOMETHING:
        hint = "[Something] Click X, then Y";
        break;

5. Add a case in WM_COMMAND:
    - Drawing mode:
        case IDM_SOMETHING:
            mode = MODE_SOMETHING;
            break;
    - Immediate action (utility):
        case IDM_SOMETHING:
            doSomething(hwnd);
            break;

6. Add to a submenu in WinMain:
    - Appending to an existing submenu:
        AppendMenu(hExisting, MF_STRING, IDM_SOMETHING, "Something");
    - Creating a new submenu (don't forget to attach it to hMenu):
        HMENU hSomething = CreatePopupMenu();
        AppendMenu(hSomething, MF_STRING, IDM_SOMETHING, "Something");
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSomething, "My Menu");
*/

// ---- Menu IDs ----
// Clipping
#define IDM_RECT_POINT 101
#define IDM_RECT_LINE 102
#define IDM_SQUARE_POINT 103
#define IDM_SQUARE_LINE 104
#define IDM_CIRCLE_POINT 105
#define IDM_CIRCLE_LINE 106
#define IDM_RECT_POLYGON 107
#define IDM_SQUARE_POLYGON 108

// Ellipse
#define IDM_ELLIPSE_DIRECT 201
#define IDM_ELLIPSE_POLAR 202
#define IDM_ELLIPSE_MIDPOINT 203

// Curves
#define IDM_HERMIT 301
#define IDM_CARDINAL 302

// Fill
#define IDM_CIRCLE_QUARTER 401
#define IDM_CIRCLE_LINE_FILL 402
#define IDM_POLYGON_CONVEX 403
#define IDM_POLYGON_GENERAL 404
#define IDM_FLOODFILL_REC 405
#define IDM_FLOODFILL_ITR 406
#define IDM_FILL_SQUARE_HERMIT 407
#define IDM_FILL_RECT_BEZIER 408

// Circle
#define IDM_CIRCLE_DIRECT 501
#define IDM_CIRCLE_POLAR 502
#define IDM_CIRCLE_POLAR_ITERATIVE 503
#define IDM_CIRCLE_MIDPOINT 504
#define IDM_CIRCLE_MIDPOINT_MODIFIED 505

// Files
#define IDM_CLEAR 601
#define IDM_SAVE 602
#define IDM_LOAD 603

// Line
#define IDM_LINE_DDA 701
#define IDM_LINE_MIDPOINT 702
#define IDM_LINE_PARAMETRIC 703

// Preferences
#define IDM_COLOR 801
#define IDM_MOUSE 802
#define IMD_BG_COLOR 803

// Custom shapes
#define IDM_SMILE 901
#define IDM_SAD 902

// ---- Global state ----
AppMode mode = MODE_NONE;
Point pts[8];
int clickCount = 0;

vector<Point> cardinalPts;

void UpdateTitle(HWND hwnd)
{
    const char *name = "Graphics App";
    const char *hint = "";
    switch (mode)
    {
    case MODE_RECT_POINT:
        hint = "[Rect-Point] Click 2 corners, then a point";
        break;
    case MODE_RECT_LINE:
        hint = "[Rect-Line]  Click 2 corners, then 2 line endpoints";
        break;
    case MODE_SQUARE_POINT:
        hint = "[Sq-Point]   Click 2 pts for square, then a point";
        break;
    case MODE_SQUARE_LINE:
        hint = "[Sq-Line]    Click 2 pts for square, then 2 line endpoints";
        break;
    case MODE_CIRCLE_POINT:
        hint = "[Circ-Point] Click center, radius-pt, then a point";
        break;
    case MODE_CIRCLE_LINE:
        hint = "[Circ-Line]  Click center, radius-pt, then 2 line endpoints";
        break;
    case MODE_ELLIPSE_DIRECT:
        hint = "[Ellipse-Direct]  Click center, then X-radius pt, then Y-radius pt";
        break;
    case MODE_ELLIPSE_POLAR:
        hint = "[Ellipse-Polar]   Click center, then X-radius pt, then Y-radius pt";
        break;
    case MODE_ELLIPSE_MIDPOINT:
        hint = "[Ellipse-Midpt]   Click center, then X-radius pt, then Y-radius pt";
        break;
    case MODE_HERMIT:
        hint = "[Hermit]    Click P1, T1-end, P2, T2-end";
        break;
    case MODE_CARDINAL:
        hint = "[Cardinal]   Left-click to add points, Right-click to draw";
        break;
    case MODE_CIRCLE_DIRECT:
        hint = "[Circle-Direct]   Click center, then radius-pt";
        break;
    case MODE_CIRCLE_POLAR:
        hint = "[Circle-Polar]    Click center, then radius-pt";
        break;
    case MODE_CIRCLE_POLAR_ITERATIVE:
        hint = "[Circle-IterPolar] Click center, then radius-pt";
        break;
    case MODE_CIRCLE_MIDPOINT:
        hint = "[Circle-Midpoint] Click center, then radius-pt";
        break;
    case MODE_CIRCLE_MIDPOINT_MODIFIED:
        hint = "[Circle-ModMidpt] Click center, then radius-pt";
        break;
    case MODE_CIRCLE_QUARTER:
        hint = "[Circ-Quarter]    Click center, radius-pt, quarter-pt";
        break;
    case MODE_CIRCLE_LINE_FILL:
        hint = "[Circ-LineFill]   Click center, radius-pt, quarter-pt";
        break;
    case MODE_FLOODFILL_REC:
        hint = "[Floodfill recursive]   Click area to fill";
        break;
    case MODE_FLOODFILL_ITR:
        hint = "[Floodfill Itrative]   Click area to fill";
        break;
    case MODE_LINE_DDA:
        hint = "[Line DDA]   Click point 1, point 2";
        break;
    case MODE_LINE_MIDPOINT:
        hint = "[Line Midpoint]   Click point 1, point 2";
        break;
    case MODE_LINE_PARAMETRIC:
        hint = "[Line PARAMETRIC]   Click point 1, point 2";
        break;

    case MODE_FILL_SQUARE_HERMIT:
        hint = "[Fill Square Hermit] Click top-left, then bottom-right";
        break;
    case MODE_FILL_RECT_BEZIER:
        hint = "[Fill Rect Bezier] Click top-left, then bottom-right";
        break;
    case MODE_RECT_POLYGON:
        hint = "[Rect-Polygon] Click 2 corners, then polygon vertices, Right-click to clip";
        break;
    case MODE_SQUARE_POLYGON:
        hint = "[Sq-Polygon] Click 2 pts for square, then polygon vertices, Right-click to clip";
        break;

    case MODE_SMILE:
        hint = "[Smile :)]";
        break;

    case MODE_SAD:
        hint = "[Sad :(]";
        break;

    default:
        hint = "Select a mode from the menus";
        break;
    }
    char title[256];
    wsprintfA(title, "%s  |  %s  (click %d)", name, hint, clickCount);
    SetWindowTextA(hwnd, title);
}

LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lp), my = HIWORD(lp);
        pts[clickCount].x = mx;
        pts[clickCount].y = my;
        clickCount++;
        UpdateTitle(hwnd);

        HDC hdc = GetDC(hwnd);

        if (mode == MODE_RECT_POINT)
        {
            if (clickCount == 2)
                Rectangle(hdc, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
            else if (clickCount == 3)
            {
                int xl = min(pts[0].x, pts[1].x), xr = max(pts[0].x, pts[1].x);
                int yt = min(pts[0].y, pts[1].y), yb = max(pts[0].y, pts[1].y);
                PointClipping(hdc, mx, my, xl, yt, xr, yb, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_RECT_LINE)
        {
            if (clickCount == 2)
                Rectangle(hdc, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
            else if (clickCount == 4)
            {
                int xl = min(pts[0].x, pts[1].x), xr = max(pts[0].x, pts[1].x);
                int yt = min(pts[0].y, pts[1].y), yb = max(pts[0].y, pts[1].y);
                CohenSutherlandClip(hdc, pts[2].x, pts[2].y, pts[3].x, pts[3].y,
                                    xl, yt, xr, yb, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_SQUARE_POINT)
        {
            if (clickCount == 2)
            {
                int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                Rectangle(hdc, pts[0].x, pts[0].y, pts[0].x + side, pts[0].y + side);
            }
            else if (clickCount == 3)
            {
                int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                PointClipping(hdc, mx, my, pts[0].x, pts[0].y,
                              pts[0].x + side, pts[0].y + side, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_CIRCLE_DIRECT || mode == MODE_CIRCLE_POLAR ||
                 mode == MODE_CIRCLE_POLAR_ITERATIVE || mode == MODE_CIRCLE_MIDPOINT ||
                 mode == MODE_CIRCLE_MIDPOINT_MODIFIED)
        {
            if (clickCount == 2)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                if (mode == MODE_CIRCLE_DIRECT)
                    circleDirect(hdc, xc, yc, r, drawColor);
                else if (mode == MODE_CIRCLE_POLAR)
                    circlePolar(hdc, xc, yc, r, drawColor);
                else if (mode == MODE_CIRCLE_POLAR_ITERATIVE)
                    circleIterativePolar(hdc, xc, yc, r, drawColor);
                else if (mode == MODE_CIRCLE_MIDPOINT)
                {
                    circleMidpoint(hdc, xc, yc, r, drawColor);
                    Shape s;
                    s.type = MODE_CIRCLE_MIDPOINT;
                    s.color = drawColor;
                    Point rad;
                    rad.x = r;
                    rad.y = 0;
                    s.pts.push_back(pts[0]);
                    s.pts.push_back(rad);
                    shapes.push_back(s);
                }
                else
                    circleModifiedMidPoint(hdc, xc, yc, r, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_SQUARE_LINE)
        {
            if (clickCount == 2)
            {
                int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                Rectangle(hdc, pts[0].x, pts[0].y, pts[0].x + side, pts[0].y + side);
            }
            else if (clickCount == 4)
            {
                int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                CohenSutherlandClip(hdc, pts[2].x, pts[2].y, pts[3].x, pts[3].y,
                                    pts[0].x, pts[0].y,
                                    pts[0].x + side, pts[0].y + side, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_CIRCLE_POINT)
        {
            if (clickCount == 2)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                Ellipse(hdc, xc - r, yc - r, xc + r, yc + r);
            }
            else if (clickCount == 3)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                circlePointClipping(hdc, mx, my, xc, yc, r, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_CIRCLE_LINE)
        {
            if (clickCount == 2)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                Ellipse(hdc, xc - r, yc - r, xc + r, yc + r);
            }
            else if (clickCount == 4)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                circleLineClipping(hdc, pts[2].x, pts[2].y, pts[3].x, pts[3].y,
                                   xc, yc, r, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_ELLIPSE_DIRECT || mode == MODE_ELLIPSE_POLAR || mode == MODE_ELLIPSE_MIDPOINT)
        {
            if (clickCount == 3)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int rx = abs(pts[1].x - xc), ry = abs(pts[2].y - yc);
                COLORREF col = (mode == MODE_ELLIPSE_DIRECT)  ? drawColor
                               : (mode == MODE_ELLIPSE_POLAR) ? drawColor
                                                              : drawColor;
                if (mode == MODE_ELLIPSE_DIRECT)
                    directEllipse(hdc, rx, ry, xc, yc, col);
                else if (mode == MODE_ELLIPSE_POLAR)
                    polarEllipse(hdc, rx, ry, xc, yc, col);
                else
                    midpointEllipse(hdc, rx, ry, xc, yc, col);
                clickCount = 0;
            }
        }

        else if (mode == MODE_HERMIT)
        {
            // Draw small marker
            SetPixel(hdc, mx, my, drawColor);
            if (clickCount == 4)
            {
                Point p1{pts[0].x, pts[0].y};
                // Tangent vectors = difference from previous point, scaled
                Point T1{(pts[1].x - pts[0].x) * 3, (pts[1].y - pts[0].y) * 3};
                Point p2{pts[2].x, pts[2].y};
                Point T2{(pts[3].x - pts[2].x) * 3, (pts[3].y - pts[2].y) * 3};
                hermit(hdc, p1, T1, p2, T2, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_CARDINAL)
        {
            Point p;
            p.x = mx;
            p.y = my;
            cardinalPts.push_back(p);
            SetPixel(hdc, mx, my, drawColor);
            SetPixel(hdc, mx + 1, my, drawColor);
            SetPixel(hdc, mx, my + 1, drawColor);
            clickCount = (int)cardinalPts.size();
        }

        else if (mode == MODE_CIRCLE_QUARTER || mode == MODE_CIRCLE_LINE_FILL)
        {
            if (clickCount == 2)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                circleMidpoint(hdc, xc, yc, r, drawColor);
            }
            else if (clickCount == 3)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                int q = getQuarter(xc, yc, mx, my);
                if (mode == MODE_CIRCLE_QUARTER)
                    circleFillCircle(hdc, xc, yc, r, q, drawColor);
                else
                    lineFillCircle(hdc, xc, yc, r, q, drawColor);
                clickCount = 0;
            }
        }

        else if (mode == MODE_POLYGON_CONVEX || mode == MODE_POLYGON_GENERAL)
        {
            Point p;
            p.x = mx;
            p.y = my;
            cardinalPts.push_back(p);
            SetPixel(hdc, mx, my, drawColor);
            SetPixel(hdc, mx + 1, my, drawColor);
            SetPixel(hdc, mx, my + 1, drawColor);
        }

        else if (mode == MODE_FLOODFILL_REC || mode == MODE_FLOODFILL_ITR)
        {

            COLORREF fillColor = drawColor;
            COLORREF targetColor = GetPixel(hdc, mx, my);

            if (mode == MODE_FLOODFILL_REC)
            {
                floodfillRec(hdc, mx, my, targetColor, fillColor);
            }
            else
            {
                floodfillIterative(hdc, mx, my, targetColor, fillColor);
            }

            clickCount = 0;
        }

        else if (mode == MODE_LINE_DDA || mode == MODE_LINE_MIDPOINT || mode == MODE_LINE_PARAMETRIC)
        {
            if (clickCount == 2)
            {
                int x1 = pts[0].x, y1 = pts[0].y;
                int x2 = pts[1].x, y2 = pts[1].y;

                if (mode == MODE_LINE_DDA)
                    lineDDA(hdc, x1, y1, x2, y2, drawColor);
                else if (mode == MODE_LINE_MIDPOINT)
                    lineMidPoint(hdc, x1, y1, x2, y2, drawColor);
                else
                    lineParametric(hdc, x1, y1, x2, y2, drawColor);

                clickCount = 0;
            }
        }

        else if (mode == MODE_FILL_SQUARE_HERMIT || mode == MODE_FILL_RECT_BEZIER)
        {
            if (clickCount == 2)
            {
                int x = min(pts[0].x, pts[1].x);
                int y = min(pts[0].y, pts[1].y);
                int w = abs(pts[1].x - pts[0].x);
                int h = abs(pts[1].y - pts[0].y);

                if (mode == MODE_FILL_SQUARE_HERMIT)
                {
                    int side = min(w, h);
                    Rectangle(hdc, x, y, x + side, y + side); // draw square outline, not rect
                    curveFillSquare(hdc, Point(x, y), side, drawColor);
                }
                else
                {
                    Rectangle(hdc, x, y, x + w, y + h);
                    curveFillRect(hdc, Point(x, y), w, h, drawColor);
                }
                clickCount = 0;
            }
        }

        else if (mode == MODE_RECT_POLYGON || mode == MODE_SQUARE_POLYGON)
        {
            if (clickCount == 2)
            {
                // Draw the clipping window
                if (mode == MODE_RECT_POLYGON)
                    Rectangle(hdc, pts[0].x, pts[0].y, pts[1].x, pts[1].y);
                else
                {
                    int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                    Rectangle(hdc, pts[0].x, pts[0].y, pts[0].x + side, pts[0].y + side);
                }
            }
            else if (clickCount > 2)
            {
                // Accumulate polygon vertices in cardinalPts
                Point p;
                p.x = mx;
                p.y = my;
                cardinalPts.push_back(p);
                SetPixel(hdc, mx, my, drawColor);
                SetPixel(hdc, mx + 1, my, drawColor);
                SetPixel(hdc, mx, my + 1, drawColor);
            }
        }

        else if (mode == MODE_SMILE || mode == MODE_SAD)
        {
            if (clickCount == 1)
            {
                if (mode == MODE_SMILE)
                    drawHappy(hdc, mx, my);
                else
                    // add code here Abdelrahman
                    drawSad(hdc, mx, my);

                clickCount = 0;
            }
        }

        UpdateTitle(hwnd);
        return 0;
    }

    // Right button to stop Cardinal spline
    case WM_RBUTTONDOWN:
    {
        if (mode == MODE_CARDINAL && cardinalPts.size() >= 2)
        {
            HDC hdc = GetDC(hwnd);
            cardinalSplineCurve(hdc, cardinalPts, 0.5, drawColor);
            ReleaseDC(hwnd, hdc);
            cardinalPts.clear();
            clickCount = 0;
            UpdateTitle(hwnd);
        }
        if ((mode == MODE_POLYGON_CONVEX || mode == MODE_POLYGON_GENERAL) && cardinalPts.size() >= 3)
        {
            HDC hdc = GetDC(hwnd);
            bool convex = isConvex(cardinalPts);

            if (mode == MODE_POLYGON_CONVEX)
            {
                if (convex)
                {
                    convexFill(hdc, &cardinalPts[0], cardinalPts.size(), drawColor);
                }
                else
                {
                    MessageBox(hwnd, "Shape is not convex!", "Error", MB_OK | MB_ICONERROR);
                }
            }
            else if (mode == MODE_POLYGON_GENERAL)
            {
                if (!convex)
                {
                    MessageBox(hwnd, "Non-convex shape detected. Proceeding with General Fill.", "Information", MB_OK | MB_ICONINFORMATION);
                }

                nonConvFill(hdc, &cardinalPts[0], cardinalPts.size(), drawColor);
            }

            drawPolygon(hdc, cardinalPts, drawColor);
            cardinalPts.clear();
            ReleaseDC(hwnd, hdc);
        }
        if ((mode == MODE_RECT_POLYGON || mode == MODE_SQUARE_POLYGON) && clickCount >= 2 && cardinalPts.size() >= 3)
        {
            HDC hdc = GetDC(hwnd);

            int xl, yt, xr, yb;
            if (mode == MODE_RECT_POLYGON)
            {
                xl = min(pts[0].x, pts[1].x);
                xr = max(pts[0].x, pts[1].x);
                yt = min(pts[0].y, pts[1].y);
                yb = max(pts[0].y, pts[1].y);
            }
            else
            {
                int side = min(abs(pts[1].x - pts[0].x), abs(pts[1].y - pts[0].y));
                xl = pts[0].x;
                xr = pts[0].x + side;
                yt = pts[0].y;
                yb = pts[0].y + side;
            }

            vector<POINT> poly(cardinalPts.size());
            for (int i = 0; i < (int)cardinalPts.size(); i++)
            {
                poly[i].x = cardinalPts[i].x;
                poly[i].y = cardinalPts[i].y;
            }

            PolygonClip(hdc, poly.data(), poly.size(), xl, yt, xr, yb);

            cardinalPts.clear();
            clickCount = 0;
            ReleaseDC(hwnd, hdc);
            UpdateTitle(hwnd);
        }
        return 0;
    }

    // ---- Menu ----
    case WM_COMMAND:
    {
        clickCount = 0;
        cardinalPts.clear();

        switch (LOWORD(wp))
        {
        // Clipping
        case IDM_RECT_POINT:
            mode = MODE_RECT_POINT;
            break;
        case IDM_RECT_LINE:
            mode = MODE_RECT_LINE;
            break;
        case IDM_SQUARE_POINT:
            mode = MODE_SQUARE_POINT;
            break;
        case IDM_SQUARE_LINE:
            mode = MODE_SQUARE_LINE;
            break;
        case IDM_CIRCLE_POINT:
            mode = MODE_CIRCLE_POINT;
            break;
        case IDM_CIRCLE_LINE:
            mode = MODE_CIRCLE_LINE;
            break;
        case IDM_RECT_POLYGON:
            mode = MODE_RECT_POLYGON;
            break;
        case IDM_SQUARE_POLYGON:
            mode = MODE_SQUARE_POLYGON;
            break;

        // Ellipse
        case IDM_ELLIPSE_DIRECT:
            mode = MODE_ELLIPSE_DIRECT;
            break;
        case IDM_ELLIPSE_POLAR:
            mode = MODE_ELLIPSE_POLAR;
            break;
        case IDM_ELLIPSE_MIDPOINT:
            mode = MODE_ELLIPSE_MIDPOINT;
            break;

        // Circle
        case IDM_CIRCLE_DIRECT:
            mode = MODE_CIRCLE_DIRECT;
            break;
        case IDM_CIRCLE_POLAR:
            mode = MODE_CIRCLE_POLAR;
            break;
        case IDM_CIRCLE_POLAR_ITERATIVE:
            mode = MODE_CIRCLE_POLAR_ITERATIVE;
            break;
        case IDM_CIRCLE_MIDPOINT:
            mode = MODE_CIRCLE_MIDPOINT;
            break;
        case IDM_CIRCLE_MIDPOINT_MODIFIED:
            mode = MODE_CIRCLE_MIDPOINT_MODIFIED;
            break;

        // Curves
        case IDM_HERMIT:
            mode = MODE_HERMIT;
            break;
        case IDM_CARDINAL:
            mode = MODE_CARDINAL;
            break;

        // Fill
        case IDM_CIRCLE_QUARTER:
            mode = MODE_CIRCLE_QUARTER;
            break;
        case IDM_CIRCLE_LINE_FILL:
            mode = MODE_CIRCLE_LINE_FILL;
            break;

        case IDM_POLYGON_CONVEX:
            mode = MODE_POLYGON_CONVEX;
            break;

        case IDM_POLYGON_GENERAL:
            mode = MODE_POLYGON_GENERAL;
            break;
        case IDM_FLOODFILL_REC:
            mode = MODE_FLOODFILL_REC;
            break;
        case IDM_FLOODFILL_ITR:
            mode = MODE_FLOODFILL_ITR;
            break;
        case IDM_FILL_SQUARE_HERMIT:
            mode = MODE_FILL_SQUARE_HERMIT;
            break;
        case IDM_FILL_RECT_BEZIER:
            mode = MODE_FILL_RECT_BEZIER;
            break;

        // File
        case IDM_CLEAR:
            clear(hwnd, BG_COLOR);
            mode = MODE_NONE;
            break;

        case IDM_SAVE:
            // TODO Save doesnt actaually save the background color the user uses fix that please
            save();
            break;

        case IDM_LOAD:
            load(hwnd);
            break;

        case IDM_COLOR:
        {
            static COLORREF customColors[16] = {};
            CHOOSECOLOR cc = {};
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.rgbResult = drawColor;
            cc.lpCustColors = customColors;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc))
                drawColor = cc.rgbResult;
            break;
        }

        // Line
        case IDM_LINE_DDA:
            mode = MODE_LINE_DDA;
            break;
        case IDM_LINE_MIDPOINT:
            mode = MODE_LINE_MIDPOINT;
            break;
        case IDM_LINE_PARAMETRIC:
            mode = MODE_LINE_PARAMETRIC;
            break;

        // Preferences
        case IMD_BG_COLOR:
            clear(hwnd, RGB(255, 255, 255));
            break;

        case IDM_MOUSE:
        {
            static bool crossCursor = true;
            crossCursor = !crossCursor;
            HCURSOR hCursor = LoadCursor(NULL, crossCursor ? IDC_CROSS : IDC_ARROW);
            SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR)hCursor);
            break;
        }

        // Custom
        case IDM_SMILE:
        {
            mode = MODE_SMILE;
            break;
        }
        case IDM_SAD:
        {
            mode = MODE_SAD;
            break;
        }
        }

        UpdateTitle(hwnd);
        return 0;
    }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int APIENTRY WinMain(HINSTANCE h, HINSTANCE, LPSTR, int nsh)
{
    srand((unsigned)time(NULL));

    WNDCLASS wc = {};
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = h;
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "GraphicsApp";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("GraphicsApp", "Graphics App",
                             WS_OVERLAPPEDWINDOW, 0, 0, 900, 700,
                             NULL, NULL, h, 0);

    //  Build menus
    HMENU hMenu = CreateMenu();

    // Clipping submenu
    HMENU hClip = CreatePopupMenu();
    AppendMenu(hClip, MF_STRING, IDM_RECT_POINT, "Rectangle - Point");
    AppendMenu(hClip, MF_STRING, IDM_RECT_LINE, "Rectangle - Line");
    AppendMenu(hClip, MF_SEPARATOR, 0, NULL);
    AppendMenu(hClip, MF_STRING, IDM_SQUARE_POINT, "Square - Point");
    AppendMenu(hClip, MF_STRING, IDM_SQUARE_LINE, "Square - Line");
    AppendMenu(hClip, MF_SEPARATOR, 0, NULL);
    AppendMenu(hClip, MF_STRING, IDM_CIRCLE_POINT, "Circle - Point");
    AppendMenu(hClip, MF_STRING, IDM_CIRCLE_LINE, "Circle - Line");
    AppendMenu(hClip, MF_STRING, IDM_RECT_POLYGON, "Rectangle - Polygon");
    AppendMenu(hClip, MF_STRING, IDM_SQUARE_POLYGON, "Square - Polygon");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hClip, "Clipping");

    // Line submenu
    HMENU hLine = CreatePopupMenu();
    AppendMenu(hLine, MF_STRING, IDM_LINE_DDA, "Line DDA");
    AppendMenu(hLine, MF_STRING, IDM_LINE_MIDPOINT, "Line Midpoint");
    AppendMenu(hLine, MF_STRING, IDM_LINE_PARAMETRIC, "Line Parametric");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLine, "Line");

    // Ellipse submenu
    HMENU hEllipse = CreatePopupMenu();
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_DIRECT, "Direct");
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_POLAR, "Polar");
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_MIDPOINT, "Midpoint");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEllipse, "Ellipse");

    // Curves submenu
    HMENU hCurve = CreatePopupMenu();
    AppendMenu(hCurve, MF_STRING, IDM_HERMIT, "Hermit  (4 clicks)");
    AppendMenu(hCurve, MF_STRING, IDM_CARDINAL, "Cardinal (LClick=add, RClick=draw)");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hCurve, "Curves");

    // Circle submenu
    HMENU hCircle = CreatePopupMenu();
    AppendMenu(hCircle, MF_STRING, IDM_CIRCLE_DIRECT, "Direct");
    AppendMenu(hCircle, MF_STRING, IDM_CIRCLE_POLAR, "Polar");
    AppendMenu(hCircle, MF_STRING, IDM_CIRCLE_POLAR_ITERATIVE, "Polar Iterative");
    AppendMenu(hCircle, MF_STRING, IDM_CIRCLE_MIDPOINT, "Midpoint");
    AppendMenu(hCircle, MF_STRING, IDM_CIRCLE_MIDPOINT_MODIFIED, "Midpoint Modified");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hCircle, "Circle");

    // Circle Fill submenu
    HMENU hFill = CreatePopupMenu();
    AppendMenu(hFill, MF_STRING, IDM_CIRCLE_QUARTER, "Quarter Fill (pixel)");
    AppendMenu(hFill, MF_STRING, IDM_CIRCLE_LINE_FILL, "Quarter Fill (lines)");
    AppendMenu(hFill, MF_STRING, IDM_POLYGON_CONVEX, "Convex Polygon Fill");
    AppendMenu(hFill, MF_STRING, IDM_POLYGON_GENERAL, "General Polygon Fill (Non-Convex)");
    AppendMenu(hFill, MF_STRING, IDM_FLOODFILL_REC, "Flood Fill Recursive");
    AppendMenu(hFill, MF_STRING, IDM_FLOODFILL_ITR, "Flood FIll Iterative");
    AppendMenu(hFill, MF_STRING, IDM_FILL_SQUARE_HERMIT, "Fill Square (Hermit vertical)");
    AppendMenu(hFill, MF_STRING, IDM_FILL_RECT_BEZIER, "Fill Rectangle (Bezier horizontal)");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFill, "Fill");

    // File submenu
    HMENU hUtil = CreatePopupMenu();
    AppendMenu(hUtil, MF_STRING, IDM_CLEAR, "Clear");
    AppendMenu(hUtil, MF_STRING, IDM_SAVE, "Save to file");
    AppendMenu(hUtil, MF_STRING, IDM_LOAD, "Load from file");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hUtil, "File");

    // Preferences menu
    HMENU hPref = CreatePopupMenu();
    AppendMenu(hPref, MF_STRING, IMD_BG_COLOR, "back to white background");
    AppendMenu(hPref, MF_STRING, IDM_MOUSE, "Change cursor");
    AppendMenu(hPref, MF_STRING, IDM_COLOR, "Pick Color");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPref, "Preferences");

    // Custom
    HMENU hCustom = CreatePopupMenu();
    AppendMenu(hCustom, MF_STRING, IDM_SMILE, "Smile");
    AppendMenu(hCustom, MF_STRING, IDM_SAD, "Sad");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hCustom, "Custom");

    SetMenu(hwnd, hMenu);

    ShowWindow(hwnd, nsh);
    UpdateWindow(hwnd);
    UpdateTitle(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}