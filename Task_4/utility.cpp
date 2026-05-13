#include <windows.h>
#include <vector>
#include <cmath>
#include <stack>
#include <climits>
#include <algorithm>
#include <fstream>

using namespace std;

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