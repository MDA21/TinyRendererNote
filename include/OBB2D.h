#pragma once
#include <vector>
#include "vector.h"

struct OBB2D
{
    Vec2f center;
    Vec2f axis0;     // 主轴（单位）
    Vec2f axis1;     // 副轴（单位）
    Vec2f halfSize;  // 半尺寸

    // === 构建 ===
    static OBB2D from_triangle(const Vec2i& a,
        const Vec2i& b,
        const Vec2i& c);

    // === OBB → AABB（用于扫描）===
    void get_bounds(int& min_x, int& max_x,
        int& min_y, int& max_y) const;

    // === 点是否在 OBB 内 ===
    bool contains_point(const Vec2i& p) const;
};
