#include <windows.h>
#include <vector>
#include <cmath>
#include <stack>
#include <climits>
#include <algorithm>
#include <list>

#include "headers/line.h"

using namespace std;

const double PI = 3.141592653589793;
const int SCREEN_HEIGHT = 800;

struct Point
{
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

vector<vector<int>> multiply(const vector<vector<int>> &m1, const vector<vector<int>> &m2)
{
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

vector<vector<double>> multiply(const vector<vector<double>> &m1, const vector<vector<int>> &m2)
{
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

void hermit(HDC hdc, Point p1, Point T1, Point p2, Point T2, COLORREF color)
{
    // Hermit matrix
    const vector<vector<int>> H = {{1, 0, 0, 0},
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

    for (double t = 0; t < 1; t += 1.0f / n)
    {
        vector<vector<double>> VT = {{1, t, t * t, t * t * t}};
        vector<vector<double>> X = multiply(VT, C);
        SetPixel(hdc, round(X[0][0]), round(X[0][1]), RGB(255, 0, 0));
    }
}

void cardinalSplineCurve(HDC hdc, const vector<Point> &points, double c, COLORREF color)
{
    int n = points.size(); // # of points

    if (n <= 2)
        return; // Can't draw with 2 points or less

    vector<Point> T(n); // Tangents for all points except the first and last points

    for (int i = 1; i < n - 1; i++)
    {
        T[i].x = c * (points[i + 1].x - points[i - 1].x);
        T[i].y = c * (points[i + 1].y - points[i - 1].y);
    }

    // Repeat the first and last calculated tangents to include first and last points
    T[0] = T[1];
    T[n - 1] = T[n - 2];

    // Draw hermit curve from every point to the next
    for (int i = 0; i < n - 1; i++)
    {
        hermit(hdc, points[i], T[i], points[i + 1], T[i + 1], color);
    }
}

// Draw lines in only 2 octants based on the chosen quarter
void draw2lines(HDC hdc, int xc, int yc, int x, int y, int q, COLORREF color)
{
    if (q == 1)
    {
        lineMidPoint(hdc, xc + x, yc - y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc - x, xc + y, yc, color);
    }
    else if (q == 2)
    {
        lineMidPoint(hdc, xc - x, yc - y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc - x, xc - y, yc, color);
    }
    else if (q == 3)
    {
        lineMidPoint(hdc, xc - x, yc + y, xc - x, yc, color);
        lineMidPoint(hdc, xc - y, yc + x, xc - y, yc, color);
    }
    else if (q == 4)
    {
        lineMidPoint(hdc, xc + x, yc + y, xc + x, yc, color);
        lineMidPoint(hdc, xc + y, yc + x, xc + y, yc, color);
    }
}

// modified midpoint algo
void lineFillCircle(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2 * r;
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
int getQuarter(int xc, int yc, int x, int y)
{
    double angle = atan2(yc - y, x - xc); // y inverted for screen coordinates

    // Normalize the angle
    if (angle < 0)
        angle += 2 * PI;

    if (angle >= 0 && angle < PI / 2)
        return 1;
    else if (angle >= PI / 2 && angle < PI)
        return 2;
    else if (angle >= PI && angle < 3 * PI / 2)
        return 3;
    else if (angle >= 3 * PI / 2 && angle < 2 * PI)
        return 4;

    return 0; // Unexpected result
}

// Draw pixels in only 2 octants based on the chosen quarter
void draw2points(HDC hdc, int xc, int yc, int x, int y, int q, COLORREF color)
{
    if (q == 1)
    {
        SetPixel(hdc, xc + x, yc - y, color);
        SetPixel(hdc, xc + y, yc - x, color);
    }
    else if (q == 2)
    {
        SetPixel(hdc, xc - x, yc - y, color);
        SetPixel(hdc, xc - y, yc - x, color);
    }
    else if (q == 3)
    {
        SetPixel(hdc, xc - x, yc + y, color);
        SetPixel(hdc, xc - y, yc + x, color);
    }
    else if (q == 4)
    {
        SetPixel(hdc, xc + x, yc + y, color);
        SetPixel(hdc, xc + y, yc + x, color);
    }
}

// modified midpoint algo
void circleQuarter(HDC hdc, int xc, int yc, int r, int quarter, COLORREF color)
{
    int x = 0, y = r, d = 1 - r, change1 = 3, change2 = 5 - 2 * r;
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

// Recursive flood fill
void floodfillRec(HDC hdc, int x, int y, COLORREF cb, COLORREF cf)
{

    if (cb == cf)
        return;

    COLORREF c = GetPixel(hdc, x, y);

    if (c != cb)
        return;

    SetPixel(hdc, x, y, cf);

    floodfillRec(hdc, x + 1, y, cb, cf);
    floodfillRec(hdc, x - 1, y, cb, cf);
    floodfillRec(hdc, x, y + 1, cb, cf);
    floodfillRec(hdc, x, y - 1, cb, cf);
}

// Iterative flood fill
void floodfillIterative(HDC hdc, int x, int y, COLORREF cb, COLORREF cf)
{
    if (cb == cf)
        return;

    stack<Point> s;
    s.push(Point(x, y));

    while (!s.empty())
    {
        Point p = s.top();
        s.pop();
        COLORREF c = GetPixel(hdc, p.x, p.y);
        if (c == cb)
        {
            SetPixel(hdc, p.x, p.y, cf);
            s.push(Point(p.x + 1, p.y));
            s.push(Point(p.x - 1, p.y));
            s.push(Point(p.x, p.y + 1));
            s.push(Point(p.x, p.y - 1));
        }
    }
}

struct Record
{
    double xleft, xright;
    Record() : xleft(INT_MAX), xright(INT_MIN) {}
    Record(double left, double right) : xleft(left), xright(right) {}
};

// A table of intersections
typedef Record Table[SCREEN_HEIGHT];

// Initialize Table
void TableInit(Table t)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        t[i].xleft = INT_MAX;
        t[i].xright = INT_MIN;
    }
}

// Add intersections to Table
void edge2Table(Point p1, Point p2, Table t)
{
    if (p1.y == p2.y)
        return; // Horizontal edge

    if (p1.y > p2.y)
        swap(p1, p2); // We loop from p1.y to p2.y

    int y = p1.y;
    double x = p1.x;
    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y); // Slope inverse

    while (y < p2.y)
    {
        if (x < t[y].xleft)
            t[y].xleft = x;
        if (x > t[y].xright)
            t[y].xright = x;
        y++;
        x += mi;
    }
}

void polygon2Table(Point p[], int n, Table t)
{
    Point v1 = p[n - 1];
    for (int i = 0; i < n; i++)
    {
        Point v2 = p[i];
        edge2Table(v1, v2, t);
        v1 = v2;
    }
}

// Draw a line between every 2 intersections
void Table2Screen(HDC hdc, Table t, COLORREF c)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        if (t[i].xleft < t[i].xright)
            lineMidPoint(hdc, ceil(t[i].xleft), i, floor(t[i].xright), i, c);
    }
}

// Convex polygon fill
void convexFill(HDC hdc, Point p[], int n, COLORREF c)
{
    Table t;
    TableInit(t);            // Initialize table with +inf and -inf
    polygon2Table(p, n, t);  // Add intersections with polygon to table
    Table2Screen(hdc, t, c); // Fill polygon
}

void drawPolygon(HDC hdc, const vector<Point> &points, COLORREF color)
{
    if (points.size() < 2)
        return;
    for (size_t i = 0; i < points.size(); i++)
    {
        Point p1 = points[i];
        Point p2 = points[(i + 1) % points.size()]; // Wrap around to close the polygon
        lineMidPoint(hdc, p1.x, p1.y, p2.x, p2.y, color);
    }
}

bool isConvex(const vector<Point> &points)
{
    int n = points.size();
    if (n < 3)
        return false;

    bool negative = false;
    bool positive = false;

    for (int i = 0; i < n; i++)
    {
        Point a = points[i];
        Point b = points[(i + 1) % n];
        Point c = points[(i + 2) % n];

        // Cross product of vectors (b-a) and (c-b)
        // (bx-ax)*(cy-by) - (by-ay)*(cx-bx)
        int cross_product = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);

        if (cross_product < 0)
            negative = true;
        else if (cross_product > 0)
            positive = true;

        if (negative && positive)
            return false; // Signs changed shit..... it's concave
    }
    return true;
}

struct TableListNode
{
    int y;
    double x, mi;
    TableListNode(double x, int y, double mi) : x(x), y(y), mi(mi) {}
};

typedef list<TableListNode> TableList[SCREEN_HEIGHT];

void TableListInit(TableList tl)
{
    for (int i = 0; i < SCREEN_HEIGHT; i++)
    {
        tl[i].clear();
    }
}

// Add intersections to Table
void nonConvEdge2Table(Point p1, Point p2, TableList t)
{
    if (p1.y == p2.y)
        return; // Horizontal edge

    if (p1.y > p2.y)
        swap(p1, p2); // We loop from p1.y to p2.y

    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y); // Slope inverse

    TableListNode node = TableListNode((double)p1.x, p2.y, mi);
    t[p1.y].push_back(node);
}

void nonConvPolygon2Table(Point p[], int n, TableList t)
{
    Point v1 = p[n - 1];
    for (int i = 0; i < n; i++)
    {
        Point v2 = p[i];
        nonConvEdge2Table(v1, v2, t);
        v1 = v2;
    }
}

void nonConvTable2Screen(HDC hdc, TableList t, COLORREF c)
{
    list<TableListNode> active;
    int y = 0;

    // Find first valid scanline
    while (y < SCREEN_HEIGHT && t[y].empty())
        y++;
    if (y == SCREEN_HEIGHT)
        return; // No edges found

    active = t[y];

    while (!active.empty())
    {
        // Sort active based on x
        active.sort([](const TableListNode &n1, const TableListNode &n2)
                    { return n1.x < n2.x; });

        for (auto it = active.begin(); it != active.end(); it++)
        {
            int x1 = ceil(it->x), y1 = y;

            it++; // Next node
            if (it == active.end())
                break;

            int x2 = floor(it->x), y2 = y;

            lineMidPoint(hdc, x1, y1, x2, y2, c);
        }

        y++;
        // Delete nodes from active having node.y == y
        auto it = active.begin();
        while (it != active.end())
        {
            if (it->y == y)
                it = active.erase(it);
            else
                it++;
        }

        // Update x
        for (auto it = active.begin(); it != active.end(); it++)
        {
            it->x += it->mi;
        }
        active.insert(active.begin(), t[y].begin(), t[y].end());
    }
}

bool isNonConvex(const vector<Point> &points)
{
    if (points.size() < 3)
        return false;
    return !isConvex(points);
}

void nonConvFill(HDC hdc, Point p[], int n, COLORREF c)
{
    TableList t;
    TableListInit(t);
    nonConvPolygon2Table(p, n, t);  // Add intersections with polygon to table
    nonConvTable2Screen(hdc, t, c); // Fill polygon
}
