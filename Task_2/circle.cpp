#include <windows.h>
void draw8points(HDC hdc, int xc, int yc, int x, int y, COLORREF c)
{
    SetPixel(hdc, xc+x, yc+y, c);
    SetPixel(hdc, xc-x, yc+y, c);
    SetPixel(hdc, xc+x, yc-y, c);
    SetPixel(hdc, xc-x, yc-y, c);
    SetPixel(hdc, xc+y, yc+x, c);
    SetPixel(hdc, xc-y, yc+x, c);
    SetPixel(hdc, xc+y, yc-x, c);
    SetPixel(hdc, xc-y, yc-x, c);
}
void drawCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw8points(hdc, xc, yc, x, y, c);
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
        draw8points(hdc, xc, yc, x, y, c);
    }
}