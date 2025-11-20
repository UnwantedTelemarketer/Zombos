#pragma once
#include <vector>
#include <queue>
#include <cmath>
#include <limits>

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};
