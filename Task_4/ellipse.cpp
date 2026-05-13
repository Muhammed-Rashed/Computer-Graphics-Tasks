#include <windows.h>
#include <vector>
#include <cmath>
#include <stack>
#include <climits>
#include <algorithm>

using namespace std;

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