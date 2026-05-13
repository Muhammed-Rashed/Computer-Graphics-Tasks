#ifndef CIRCLE_H
#define CIRCLE_H
#include <windows.h>
void circleDirect(HDC hdc, int xc, int yc, int r, COLORREF color);
void circlePolar(HDC hdc, int xc, int yc, int r, COLORREF color);
void circleIterativePolar(HDC hdc, int xc, int yc, int r, COLORREF color);
void circleMidpoint(HDC hdc, int xc, int yc, int r, COLORREF color);
void circleModifiedMidPoint(HDC hdc, int xc, int yc, int r, COLORREF color);
#endif