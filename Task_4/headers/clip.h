#ifndef CLIP_H
#define CLIP_H
#include <windows.h>
void PointClipping(HDC hdc, int x, int y, int xleft, int ytop, int xright, int ybottom, COLORREF color);
void CohenSutherlandClip(HDC hdc, int xs, int ys, int xe, int ye, int xleft, int ytop, int xright, int ybottom, COLORREF color);
void PolygonClip(HDC hdc, POINT *p, int n, int xleft, int ytop, int xright, int ybottom);
void circlePointClipping(HDC hdc, int x, int y, int xc, int yc, int r, COLORREF color);
void circleLineClipping(HDC hdc, int x1, int y1, int x2, int y2, int xc, int yc, int r, COLORREF color);
#endif