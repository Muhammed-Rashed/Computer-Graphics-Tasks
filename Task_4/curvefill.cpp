#include <windows.h>
#include <vector>
#include <cmath>

using namespace std;

const double PI = 3.141592653589793;

struct Point {
    int x = 0, y = 0;
};

vector<vector<int>> multiply(const vector<vector<int>> &m1, const vector<vector<int>> &m2) {
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

vector<vector<double>> multiply(const vector<vector<double>> &m1, const vector<vector<int>> &m2) {
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

void hermite(HDC hdc, Point p1, Point T1, Point p2, Point T2) {
    // Hermite matrix
    const vector<vector<int>> H = { {1, 0, 0, 0},
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

    for(double t = 0; t < 1; t += 1.0f/n) {
        vector<vector<double>> VT = {{1, t, t*t, t*t*t}};
        vector<vector<double>> X = multiply(VT, C);
        SetPixel(hdc, round(X[0][0]), round(X[0][1]), RGB(255,0,0));
    }

}

void cardinalSplineCurve(HDC hdc, const vector<Point> &points, double c) {
    int n = points.size(); // # of points
    
    if(n <= 2) return; // Can't draw with 2 points or less

    vector<Point> T(n); // Tangents for all points except the first and last points
    
    for(int i = 1; i < n-1; i++) {
        T[i].x = c * (points[i+1].x - points[i-1].x);
        T[i].y = c * (points[i+1].y - points[i-1].y);
    }

    // Repeat the first and last calculated tangents to include first and last points
    T[0] = T[1];
    T[n-1] = T[n-2];

    // Draw hermite curve from every point to the next
    for(int i = 0; i < n-1; i++) {
        hermite(hdc, points[i], T[i], points[i+1], T[i+1]);
    }
}

// Draw lines in only 2 octants based on the chosen quarter
void draw2lines(HDC hdc, int xc, int yc, int x, int y, int q, COLORREF color)
{
    if(q == 1) {
        lineMidPoint(hdc, xc + x, yc - y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc - x, xc + y, yc, color);
    }
    else if(q == 2) {
        lineMidPoint(hdc, xc - x, yc - y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc - x, xc - y, yc, color);
    }
    else if(q == 3) {
        lineMidPoint(hdc, xc - x, yc + y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc + x, xc - y, yc, color);
    }
    else if(q == 4) {
        lineMidPoint(hdc, xc + x, yc + y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc + x, xc + y, yc, color);
    }
}

// modified midpoint algo
void lineFillCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw2lines(hdc, xc, yc, x, y, quarter, color);
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
        draw2lines(hdc, xc, yc, x, y, quarter, color);
    }
}

// Get the chosen circle quarter based on angle with the center
int getQuarter(int xc, int yc, int x, int y) {
    double angle = atan2(yc-y, x-xc); // y inverted for screen coordinates

    // Normalize the angle
    if(angle < 0) angle += 2 * PI;

    if(angle >= 0 && angle < PI/2) return 1;
    else if(angle >= PI/2 && angle < PI) return 2;
    else if(angle >= PI && angle < 3*PI/2) return 3;
    else if(angle >= 3*PI/2 && angle < 2*PI) return 4;

    return 0; // Unexpected result
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

// modified midpoint algo
void circleQuarter(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2*r;
    draw2points(hdc, xc, yc, x, y, quarter, color);
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
        draw2points(hdc, xc, yc, x, y, quarter, color);
    }
}

void circleFillCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color)
{
    // Filling between the 2 circles
    while (r--)
    {
        circleQuarter(hdc, xc, yc, r, quarter, color);
    }
}
