#ifndef CURVEFILL_H
#define CURVEFILL_H
#include <windows.h>
#include <vector>

struct Point {
    int x, y;
    Point(int x = 0, int y = 0):x(x), y(y){}
};

void hermit(HDC hdc, Point p1, Point T1, Point p2, Point T2, COLORREF color);
void cardinalSplineCurve(HDC hdc, const std::vector<Point> &points, double c, COLORREF color);
void circleFillCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color);
void lineFillCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color);

int getQuarter(int xc, int yc, int x, int y);

void convexFill(HDC hdc, Point p[], int n, COLORREF c);
void drawPolygon(HDC hdc, const std::vector<Point>& points, COLORREF color);
bool isConvex(const std::vector<Point>& points);

#endif