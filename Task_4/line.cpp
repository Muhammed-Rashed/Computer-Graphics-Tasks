#include <windows.h>
#include <vector>
#include <cmath>
#include <stack>
#include <climits>
#include <algorithm>

using namespace std;

void swap(int &x1, int &x2, int &y1, int &y2)
{
    int temp = x1;
    int temp2 = y1;
    x1 = x2;
    y1 = y2;
    x2 = temp;
    y2 = temp2;
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
