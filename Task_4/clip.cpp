#include <windows.h>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

//  Point Clipping
void PointClipping(HDC hdc, int x, int y, int xleft, int ytop, int xright, int ybottom, COLORREF color)
{
    if (x >= xleft && x <= xright && y >= ytop && y <= ybottom)
    {
        SetPixel(hdc, x, y, color);
    }
}


// Line Clipping
union OutCode
{
    unsigned All : 4;
    struct {unsigned left : 1, top : 1, right : 1, bottom : 1;};
};

OutCode GetOutCode(double x, double y, int xleft, int ytop, int xright, int ybottom)
{
    OutCode out;
    out.All = 0;
    if (x < xleft) out.left = 1; else if (x > xright) out.right = 1;
    if (y < ytop) out.top = 1; else if (y > ybottom) out.bottom = 1;
    return out;
}

void VIntersect(double xs, double ys, double xe, double ye, int x, double* xi, double* yi)
{
    *xi = x;
    *yi = ys + (x - xs) * (ye - ys) / (xe - xs);
}

void HIntersect(double xs, double ys, double xe, double ye, int y, double* xi, double* yi)
{
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

void CohenSutherlandClip(HDC hdc, int xs, int ys, int xe, int ye, int xleft, int ytop, int xright, int ybottom, COLORREF color)
{
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    OutCode out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
    OutCode out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);

    while ((out1.All || out2.All) && !(out1.All & out2.All))
    {
        double xi, yi;
        if (out1.All)
        {
            if (out1.left) VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out1.top) HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out1.right) VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            x1 = xi;
            y1 = yi;
            out1 = GetOutCode(x1, y1, xleft, ytop, xright, ybottom);
        }
        else
        {
            if (out2.left) VIntersect(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (out2.top) HIntersect(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (out2.right) VIntersect(x1, y1, x2, y2, xright, &xi, &yi);
            else HIntersect(x1, y1, x2, y2, ybottom, &xi, &yi);
            x2 = xi;
            y2 = yi;
            out2 = GetOutCode(x2, y2, xleft, ytop, xright, ybottom);
        }
    }

    if (!out1.All && !out2.All)
    {
        MoveToEx(hdc, round(x1), round(y1), NULL);
        LineTo(hdc, round(x2), round(y2));
    }
}

// Polygon Clipping
struct Vertex
{
    double x,y;
    Vertex(int x1=0,int y1=0)
    {
        x=x1;
        y=y1;
    }
};
typedef vector<Vertex> VertexList;
typedef bool (*IsInFunc)(Vertex& v,int edge);
typedef Vertex (*IntersectFunc)(Vertex& v1, Vertex& v2, int edge);


VertexList ClipWithEdge(VertexList p, int edge, IsInFunc In, IntersectFunc Intersect)
{
    VertexList OutList;
    if(p.empty()) return VertexList();
    Vertex v1 = p[p.size()-1];
    bool v1_in = In(v1,edge);
    for(int i=0;i<(int)p.size();i++)
    {
        Vertex v2 = p[i];
        bool v2_in = In(v2,edge);
        if(!v1_in && v2_in)
        {
            OutList.push_back(Intersect(v1,v2,edge));
            OutList.push_back(v2);
        }

        else if(v1_in && v2_in)
        {
            OutList.push_back(v2);
        }

        else if(v1_in)
        {
            OutList.push_back(Intersect(v1,v2,edge));
        }

        v1 = v2;
        v1_in = v2_in;
    }
    return OutList;
}

bool InLeft(Vertex& v,int edge)
{
    return v.x >= edge;
}

bool InRight(Vertex& v,int edge)
{
    return v.x <= edge;
}

bool InTop(Vertex& v,int edge)
{
    return v.y >= edge;
}

bool InBottom(Vertex& v,int edge)
{
    return v.y <= edge;
}

Vertex VIntersect(Vertex& v1, Vertex& v2, int xedge)
{
    Vertex res;
    res.x = xedge;
    res.y = v1.y + (xedge - v1.x) * (v2.y - v1.y) / (v2.x - v1.x);
    return res;
}

Vertex HIntersect(Vertex& v1, Vertex& v2, int yedge)
{
    Vertex res;
    res.y = yedge;
    res.x = v1.x + (yedge - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
    return res;
}

void PolygonClip(HDC hdc, POINT *p, int n, int xleft, int ytop, int xright, int ybottom)
{
    VertexList vlist;
    for(int i=0;i<n;i++)
    {
        vlist.push_back(Vertex(p[i].x,p[i].y));
    }

    vlist = ClipWithEdge(vlist, xleft, InLeft, VIntersect);
    vlist = ClipWithEdge(vlist, ytop, InTop, HIntersect);
    vlist = ClipWithEdge(vlist, xright, InRight, VIntersect);
    vlist = ClipWithEdge(vlist, ybottom, InBottom, HIntersect);
    if(vlist.empty()) return;
    Vertex v1 = vlist[vlist.size()-1];

    for(int i=0;i<(int)vlist.size();i++)
    {
        Vertex v2 = vlist[i];
        MoveToEx(hdc, round(v1.x), round(v1.y), NULL);
        LineTo(hdc, round(v2.x), round(v2.y));
        v1 = v2;
    }
}


// Circle Window Clipping
bool insideCircle(int x, int y, int xc, int yc, int r)
{
    int dx = x - xc;
    int dy = y - yc;
    return (dx * dx + dy * dy) <= r * r;
}

void circlePointClipping(HDC hdc, int x, int y, int xc, int yc, int r, COLORREF color)
{
    if(insideCircle(x,y,xc,yc,r))
    {
        SetPixel(hdc,x,y,color);
    }
}

void circleLineClipping(HDC hdc, int x1, int y1, int x2, int y2, int xc, int yc, int r, COLORREF color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    double xInc = (double)dx / steps;
    double yInc = (double)dy / steps;

    double x = x1;
    double y = y1;

    for(int i = 0; i <= steps; i++)
    {
        if(insideCircle(round(x), round(y), xc,yc,r))
        {
            SetPixel(hdc, round(x), round(y), color);
        }
        x += xInc;
        y += yInc;
    }
}