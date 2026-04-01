#include <windows.h>
void draw7points(HDC hdc, int xc, int yc, int x, int y)
{
    SetPixel(hdc, xc - x, yc + y, RGB(0, 255, 0));
    SetPixel(hdc, xc + x, yc - y, RGB(0, 0, 255));
    SetPixel(hdc, xc - x, yc - y, RGB(255, 0, 0));
    SetPixel(hdc, xc + y, yc + x, RGB(165, 42, 42));
    SetPixel(hdc, xc - y, yc + x, RGB(128, 0, 128));
    SetPixel(hdc, xc + y, yc - x, RGB(139, 0, 0));
    SetPixel(hdc, xc - y, yc - x, RGB(0, 0, 0));
}
void drawCircle(HDC hdc, int xc, int yc, int r)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw7points(hdc, xc, yc, x, y);
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
        draw7points(hdc, xc, yc, x, y);
    }
}

void fill(HDC hdc, int xc, int yc, int R1, int R2)
{
    for (int r = R2; r >= R1; r--)
    {
        drawCircle(hdc, xc, yc, r);
    }
}
