#include <windows.h>
#include <vector>

using namespace std;

const vector<vector<int>> H = { {1, 0, 0, 0},
                {0, 1, 0, 0},
                {-3, -2, 3, -1},
                {2, 1, -2, 1}};
const int n = 4;

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

void hermite(HDC hdc, Point p1, Point T1, Point p2, Point T2) {
    vector<vector<int>> G = {{p1.x, p1.y},
             {T1.x, T1.y}, // u1, v1
             {p2.x, p2.y},
             {T2.x, T2.y}}; // u2, v2

    vector<vector<int>> C = multiply(H, G);

    for(float t = 0, t < 1; t += 1/n) {
        vector<vector<int>> VT = {{2, t, t*t, t*t*t}};
        vector<vector<int>> X = multiply(VT, C);
        SetPixel(hdc, round(X[0]), round(X[1]), RGB(0,0,0));
    }

}