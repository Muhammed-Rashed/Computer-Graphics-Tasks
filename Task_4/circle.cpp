#include <windows.h>
#include <vector>
#include <cmath>
#include <stack>
#include <climits>
#include <algorithm>


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