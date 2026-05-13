#include <windows.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <ctime>

using namespace std;

const double PI = 3.141592653589793;
const COLORREF BG_COLOR = BG_COLOR;

// ---- Menu IDs ----
// Clipping
#define IDM_RECT_POINT 101
#define IDM_RECT_LINE 102
#define IDM_SQUARE_POINT 103
#define IDM_SQUARE_LINE 104
#define IDM_CIRCLE_POINT 105
#define IDM_CIRCLE_LINE 106

// Ellipse
#define IDM_ELLIPSE_DIRECT 201
#define IDM_ELLIPSE_POLAR 202
#define IDM_ELLIPSE_MIDPOINT 203

// Curves
#define IDM_HERMITE 301
#define IDM_CARDINAL 302

// Circle Fill
#define IDM_CIRCLE_QUARTER 401
#define IDM_CIRCLE_LINE_FILL 402

// Circle
#define IDM_CIRCLE_DIRECT 501
#define IDM_CIRCLE_POLAR 502
#define IDM_CIRCLE_POLAR_ITERATIVE 503
#define IDM_CIRCLE_MIDPOINT 504
#define IDM_CIRCLE_MIDPOINT_MODIFIED 505

// Utility
#define IDM_CLEAR 601
#define IDM_SAVE 602
#define IDM_LOAD 603

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

    // Ellipse
    MODE_ELLIPSE_DIRECT,
    MODE_ELLIPSE_POLAR,
    MODE_ELLIPSE_MIDPOINT,

    // Curves
    MODE_HERMITE,  // 4 clicks: p1, T1_endpoint, p2, T2_endpoint
    MODE_CARDINAL, // N clicks, right-click to finish

    // Circle Draw
    MODE_CIRCLE_DIRECT,
    MODE_CIRCLE_POLAR,
    MODE_CIRCLE_POLAR_ITERATIVE,
    MODE_CIRCLE_MIDPOINT,
    MODE_CIRCLE_MIDPOINT_MODIFIED,

    // Circle Fill
    MODE_CIRCLE_QUARTER,   // 3 clicks: center, radius-point, quarter-point
    MODE_CIRCLE_LINE_FILL, // 3 clicks: center, radius-point, quarter-point
};

// ---- Global state ----
AppMode mode = MODE_NONE;
POINT pts[8];
int clickCount = 0;

struct Point
{
    int x = 0, y = 0;
};
vector<Point> cardinalPts;

// ---- Utility ----
// clear function
void clear(HWND hwnd, COLORREF bgColor)
{
    HDC hdc = GetDC(hwnd);
    RECT rect;
    GetClientRect(hwnd, &rect);
    for (int x = 0; x < rect.right; x++)
    {
        for (int y = 0; y < rect.bottom; y++)
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
    for (int x = 0; x < rect.right; x++)
    {
        for (int y = 0; y < rect.bottom; y++)
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

void swap(int &x1, int &x2, int &y1, int &y2)
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

// ---- line algorithms ----
void lineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (abs(dy) <= abs(dx))
    {
        double m = (double)dy / dx;
        if (x1 > x2)
            swap(x1, x2, y1, y2);
        SetPixel(hdc, x1, y1, color);
        int x = x1;
        double y = y1;
        while (x < x2)
        {
            x++;
            y += m;
            SetPixel(hdc, x, round(y), color);
        }
    }
    else
    {
        double mi = (double)dx / dy;
        if (y1 > y2)
            swap(x1, x2, y1, y2);
        SetPixel(hdc, x1, y1, color);
        int y = y1;
        double x = x1;
        while (y < y2)
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

    for (double t = 0; t <= 1; t += 1.0 / steps)
    {
        int x = round(x1 + t * dx);
        int y = round(y1 + t * dy);

        SetPixel(hdc, x, y, color);
    }
}

// Draw lines in only 2 octants based on the chosen quarter
void draw2lines(HDC hdc, int xc, int yc, int x, int y, int q, COLORREF color)
{
    if (q == 1)
    {
        lineMidPoint(hdc, xc + x, yc - y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc - x, xc + y, yc, color);
    }
    else if (q == 2)
    {
        lineMidPoint(hdc, xc - x, yc - y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc - x, xc - y, yc, color);
    }
    else if (q == 3)
    {
        lineMidPoint(hdc, xc - x, yc + y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc + x, xc - y, yc, color);
    }
    else if (q == 4)
    {
        lineMidPoint(hdc, xc + x, yc + y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc + x, xc + y, yc, color);
    }
}

// Draw pixels in only 2 octants based on the chosen quarter
void draw2points(HDC hdc, int xc, int yc, int x, int y, int q, COLORREF color)
{
    if(q == 1) {
        SetPixel(hdc, xc + x, yc - y, color);
        SetPixel(hdc, xc + y, yc - x, color);
    }
    else if(q == 2) {
        SetPixel(hdc, xc - x, yc - y, color);
        SetPixel(hdc, xc - y, yc - x, color);
    }
    else if(q == 3) {
        SetPixel(hdc, xc - x, yc + y, color);
        SetPixel(hdc, xc - y, yc + x, color);
    }
    else if(q == 4) {
        SetPixel(hdc, xc + x, yc + y, color);
        SetPixel(hdc, xc + y, yc + x, color);
    }
}

void circleQuarterDraw(HDC hdc, int xc, int yc, int r, int q, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, c1 = 3, c2 = 5 - 2 * r;
    draw2points(hdc, xc, yc, x, y, q, color);
    while (x < y)
    {
        if (d < 0)
        {
            d += c1;
            c2 += 2;
        }
        else
        {
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        draw2points(hdc, xc, yc, x, y, q, color);
    }
}

void circleFillCircle(HDC hdc, int xc, int yc, int r, int q, COLORREF color)
{
    for (int ri = r; ri >= 0; ri--)
        circleQuarterDraw(hdc, xc, yc, ri, q, color);
}

void lineFillCircle(HDC hdc, int xc, int yc, int r, int q, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, c1 = 3, c2 = 5 - 2 * r;
    draw2lines(hdc, xc, yc, x, y, q, color);
    while (x < y)
    {
        if (d < 0)
        {
            d += c1;
            c2 += 2;
        }
        else
        {
            d += c2;
            c2 += 4;
            y--;
        }
        c1 += 2;
        x++;
        draw2lines(hdc, xc, yc, x, y, q, color);
    }
}

int getQuarter(int xc, int yc, int x, int y)
{
    double angle = atan2(yc - y, x - xc);
    if (angle < 0)
        angle += 2 * PI;
    if (angle < PI / 2)
        return 1;
    if (angle < PI)
        return 2;
    if (angle < 3 * PI / 2)
        return 3;
    return 4;
}

// ---- Ellipse Algorithms ----
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
void polarEllipse(HDC hdc, int rx, int ry, int xc, int yc, COLORREF color)
{
    float x, y;
    float angle = 0;
    float step = 0.01;

    while (angle <= 2 * M_PI)
    {
        x = xc + (rx * cos(angle));
        y = yc + (ry * sin(angle));

        SetPixel(hdc, round(x), round(y), color);

        angle += step;
    }
}

// ---- Circle algorithms ----

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
    while (x > y)
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
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2 * r;
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

// ---- Clipping Algorithms ----
void PointClipping(HDC hdc, int x, int y, int xl, int yt, int xr, int yb, COLORREF color)
{
    if (x >= xl && x <= xr && y >= yt && y <= yb)
        SetPixel(hdc, x, y, color);
}

// Line Clipping
union OutCode
{
    unsigned All : 4;
    struct
    {
        unsigned left : 1, top : 1, right : 1, bottom : 1;
    };
};

OutCode GetOutCode(double x, double y, int xleft, int ytop, int xright, int ybottom)
{
    OutCode out;
    out.All = 0;
    if (x < xleft)
        out.left = 1;
    else if (x > xright)
        out.right = 1;
    if (y < ytop)
        out.top = 1;
    else if (y > ybottom)
        out.bottom = 1;
    return out;
}

void VIntersect(double xs, double ys, double xe, double ye, int x, double *xi, double *yi)
{
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}

void HIntersect(double xs, double ys, double xe, double ye, int y, double *xi, double *yi)
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
            if (out1.left)
                VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out1.top)
                HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out1.right)
                VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else
                HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            x1 = xi;
            y1 = yi;
            out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
        }
        else
        {
            if (out2.left)
                VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out2.top)
                HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out2.right)
                VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else
                HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
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
    if (insideCircle(x, y, xc, yc, r))
    {
        SetPixel(hdc, x, y, color);
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

    for (int i = 0; i <= steps; i++)
    {
        if (insideCircle(round(x), round(y), xc, yc, r))
        {
            SetPixel(hdc, round(x), round(y), color);
        }

        x += xInc;
        y += yInc;
    }
}

// ---- Curve Algorithms ----
vector<vector<int>> multiply(const vector<vector<int>> &m1, const vector<vector<int>> &m2)
{
    int r1 = m1.size();
    int c1 = m1[0].size();
    int c2 = m2[0].size();

    vector<vector<int>> C(r1, vector<int>(c2, 0));

    for (int i = 0; i < r1; i++)
        for (int j = 0; j < c2; j++)
            for (int k = 0; k < c1; k++)
                C[i][j] += m1[i][k] * m2[k][j];

    return C;
}

vector<vector<double>> multiply(const vector<vector<double>> &m1, const vector<vector<int>> &m2)
{
    int r1 = m1.size();
    int c1 = m1[0].size();
    int c2 = m2[0].size();

    vector<vector<double>> C(r1, vector<double>(c2, 0));

    for (int i = 0; i < r1; i++)
        for (int j = 0; j < c2; j++)
            for (int k = 0; k < c1; k++)
                C[i][j] += m1[i][k] * m2[k][j];

    return C;
}

void hermite(HDC hdc, Point p1, Point T1, Point p2, Point T2, COLORREF color)
{
    // Hermite matrix
    const vector<vector<int>> H = {{1, 0, 0, 0},
                                   {0, 1, 0, 0},
                                   {-3, -2, 3, -1},
                                   {2, 1, -2, 1}};

    // # of points to be drawn
    const int n = 2500;

    vector<vector<int>> G = {{p1.x, p1.y},
                             {T1.x, T1.y}, // u1, v1
                             {p2.x, p2.y},
                             {T2.x, T2.y}}; // u2, v2

    vector<vector<int>> C = multiply(H, G);

    for (double t = 0; t < 1; t += 1.0f / n)
    {
        vector<vector<double>> VT = {{1, t, t * t, t * t * t}};
        vector<vector<double>> X = multiply(VT, C);
        SetPixel(hdc, round(X[0][0]), round(X[0][1]), RGB(255, 0, 0));
    }
}

void cardinalSpline(HDC hdc, const vector<Point> &points, double c, COLORREF color)
{
    int n = points.size(); // # of points

    if (n <= 2)
        return; // Can't draw with 2 points or less

    vector<Point> T(n); // Tangents for all points except the first and last points

    for (int i = 1; i < n - 1; i++)
    {
        T[i].x = c * (points[i + 1].x - points[i - 1].x);
        T[i].y = c * (points[i + 1].y - points[i - 1].y);
    }

    // Repeat the first and last calculated tangents to include first and last points
    T[0] = T[1];
    T[n - 1] = T[n - 2];

    // Draw hermite curve from every point to the next
    for (int i = 0; i < n - 1; i++)
    {
        hermite(hdc, points[i], T[i], points[i + 1], T[i + 1], color);
    }
}

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
    case MODE_HERMITE:
        hint = "[Hermite]    Click P1, T1-end, P2, T2-end";
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
                PointClipping(hdc, mx, my, xl, yt, xr, yb, RGB(255, 0, 0));
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
                                    xl, yt, xr, yb, RGB(0, 0, 255));
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
                              pts[0].x + side, pts[0].y + side, RGB(255, 0, 255));
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
                    circleDirect(hdc, xc, yc, r, RGB(255, 0, 0));
                else if (mode == MODE_CIRCLE_POLAR)
                    circlePolar(hdc, xc, yc, r, RGB(0, 180, 0));
                else if (mode == MODE_CIRCLE_POLAR_ITERATIVE)
                    circleIterativePolar(hdc, xc, yc, r, RGB(0, 0, 255));
                else if (mode == MODE_CIRCLE_MIDPOINT)
                    circleMidpoint(hdc, xc, yc, r, RGB(200, 100, 0));
                else
                    circleModifiedMidPoint(hdc, xc, yc, r, RGB(150, 0, 200));
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
                                    pts[0].x + side, pts[0].y + side, RGB(0, 255, 0));
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
                circlePointClipping(hdc, mx, my, xc, yc, r, RGB(255, 128, 0));
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
                                   xc, yc, r, RGB(0, 128, 255));
                clickCount = 0;
            }
        }

        else if (mode == MODE_ELLIPSE_DIRECT || mode == MODE_ELLIPSE_POLAR || mode == MODE_ELLIPSE_MIDPOINT)
        {
            if (clickCount == 3)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int rx = abs(pts[1].x - xc), ry = abs(pts[2].y - yc);
                COLORREF col = (mode == MODE_ELLIPSE_DIRECT)  ? RGB(255, 0, 0)
                               : (mode == MODE_ELLIPSE_POLAR) ? RGB(0, 200, 0)
                                                              : RGB(0, 0, 255);
                if (mode == MODE_ELLIPSE_DIRECT)
                    directEllipse(hdc, rx, ry, xc, yc, col);
                else if (mode == MODE_ELLIPSE_POLAR)
                    polarEllipse(hdc, rx, ry, xc, yc, col);
                else
                    midpointEllipse(hdc, rx, ry, xc, yc, col);
                clickCount = 0;
            }
        }

        else if (mode == MODE_HERMITE)
        {
            // Draw small marker
            SetPixel(hdc, mx, my, RGB(128, 128, 128));
            if (clickCount == 4)
            {
                Point p1{pts[0].x, pts[0].y};
                // Tangent vectors = difference from previous point, scaled
                Point T1{(pts[1].x - pts[0].x) * 3, (pts[1].y - pts[0].y) * 3};
                Point p2{pts[2].x, pts[2].y};
                Point T2{(pts[3].x - pts[2].x) * 3, (pts[3].y - pts[2].y) * 3};
                hermite(hdc, p1, T1, p2, T2, RGB(200, 50, 200));
                clickCount = 0;
            }
        }

        else if (mode == MODE_CARDINAL)
        {
            Point p;
            p.x = mx;
            p.y = my;
            cardinalPts.push_back(p);
            SetPixel(hdc, mx, my, RGB(0, 150, 0));
            SetPixel(hdc, mx + 1, my, RGB(0, 150, 0));
            SetPixel(hdc, mx, my + 1, RGB(0, 150, 0));
            clickCount = (int)cardinalPts.size(); // keep count in sync
        }

        else if (mode == MODE_CIRCLE_QUARTER || mode == MODE_CIRCLE_LINE_FILL)
        {
            if (clickCount == 2)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                circleMidpoint(hdc, xc, yc, r, RGB(0, 0, 0));
            }
            else if (clickCount == 3)
            {
                int xc = pts[0].x, yc = pts[0].y;
                int r = (int)sqrt(pow(pts[1].x - xc, 2) + pow(pts[1].y - yc, 2));
                int q = getQuarter(xc, yc, mx, my);
                if (mode == MODE_CIRCLE_QUARTER)
                    circleFillCircle(hdc, xc, yc, r, q, RGB(255, 80, 0));
                else
                    lineFillCircle(hdc, xc, yc, r, q, RGB(0, 180, 180));
                clickCount = 0;
            }
        }

        ReleaseDC(hwnd, hdc);
        UpdateTitle(hwnd);
        return 0;
    }

    //  Right button to stop Cardinal spline
    case WM_RBUTTONDOWN:
    {
        if (mode == MODE_CARDINAL && cardinalPts.size() >= 2)
        {
            HDC hdc = GetDC(hwnd);
            cardinalSpline(hdc, cardinalPts, 0.5, RGB(0, 0, 180));
            ReleaseDC(hwnd, hdc);
            cardinalPts.clear();
            clickCount = 0;
            UpdateTitle(hwnd);
        }
        return 0;
    }

    //  Menu
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
        case IDM_HERMITE:
            mode = MODE_HERMITE;
            break;
        case IDM_CARDINAL:
            mode = MODE_CARDINAL;
            break;

        // Circle Fill
        case IDM_CIRCLE_QUARTER:
            mode = MODE_CIRCLE_QUARTER;
            break;
        case IDM_CIRCLE_LINE_FILL:
            mode = MODE_CIRCLE_LINE_FILL;
            break;

        // Utility
        case IDM_CLEAR:
            clear(hwnd, BG_COLOR);
            mode = MODE_NONE;
            break;
        case IDM_SAVE:
            // TODO Save doesnt actaually save the background color the user uses fix that please
            save(hwnd, BG_COLOR);
            break;
        case IDM_LOAD:
            load(hwnd);
            break;
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
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hClip, "Clipping");

    // Ellipse submenu
    HMENU hEllipse = CreatePopupMenu();
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_DIRECT, "Direct");
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_POLAR, "Polar");
    AppendMenu(hEllipse, MF_STRING, IDM_ELLIPSE_MIDPOINT, "Midpoint");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEllipse, "Ellipse");

    // Curves submenu
    HMENU hCurve = CreatePopupMenu();
    AppendMenu(hCurve, MF_STRING, IDM_HERMITE, "Hermite  (4 clicks)");
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
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFill, "Circle Fill");

    // Utility submenu
    HMENU hUtil = CreatePopupMenu();
    AppendMenu(hUtil, MF_STRING, IDM_CLEAR, "Clear");
    AppendMenu(hUtil, MF_STRING, IDM_SAVE, "Save to file");
    AppendMenu(hUtil, MF_STRING, IDM_LOAD, "Load from file");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hUtil, "Utility");

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