#include "vector.h"
#include <algorithm>
#include <cmath>

struct OBB2D {
    Vec2i center;          // OBB中心（三角形几何中心）
    Vec2i axes[2];         // 2个正交轴向（对齐三角形主轴）
    int half_extents[2];   // 每个轴向的半长度（像素）

    // 默认构造
    OBB2D() : center(Vec2i(0, 0)) {
        axes[0] = Vec2i(1, 0);
        axes[1] = Vec2i(0, 1);
        half_extents[0] = 0;
        half_extents[1] = 0;
    }

    // 从三角形3个顶点构造OBB（核心：对齐三角形主轴）
    static OBB2D from_triangle(const Vec2i& v0, const Vec2i& v1, const Vec2i& v2) {
        OBB2D obb;

        // 1. 计算三角形几何中心（OBB中心）
        obb.center = (v0 + v1 + v2) / 3;

        // 2. 计算三条边向量，找最长边（主轴方向）
        Vec2i e0 = v1 - v0;
        Vec2i e1 = v2 - v1;
        Vec2i e2 = v0 - v2;

        float len0 = e0.length_sq();
        float len1 = e1.length_sq();
        float len2 = e2.length_sq();

        Vec2i main_axis = len0 > len1 ? (len0 > len2 ? e0 : e2) : (len1 > len2 ? e1 : e2);

        // 3. 主轴归一化（转单位向量，避免长度干扰）
        float main_len = main_axis.length();
        if (main_len < 1e-8) { // 避免除零（顶点重合）
            obb.axes[0] = Vec2i(1, 0);
            obb.axes[1] = Vec2i(0, 1);
        }
        else {
            // 归一化主轴
            obb.axes[0] = Vec2i(
                static_cast<int>(main_axis.x / main_len),
                static_cast<int>(main_axis.y / main_len)
            );
            // 垂直轴向（2D中(x,y)的垂直向量是(-y,x)）
            obb.axes[1] = Vec2i(-obb.axes[0].y, obb.axes[0].x);
        }

        // 4. 投影顶点到两个轴向，计算半长
        float min_p = 1e9, max_p = -1e9; // 主轴投影极值
        float min_q = 1e9, max_q = -1e9; // 垂直轴投影极值

        auto project = [&](const Vec2i& p) {
            Vec2i dir = p - obb.center;
            float p_proj = dir.dot(obb.axes[0]); // 主轴投影
            float q_proj = dir.dot(obb.axes[1]); // 垂直轴投影

            min_p = std::min(min_p, p_proj);
            max_p = std::max(max_p, p_proj);
            min_q = std::min(min_q, q_proj);
            max_q = std::max(max_q, q_proj);
            };

        project(v0);
        project(v1);
        project(v2);

        // 5. 设置半长（取整适配像素）
        obb.half_extents[0] = static_cast<int>((max_p - min_p) / 2 + 0.5f);
        obb.half_extents[1] = static_cast<int>((max_q - min_q) / 2 + 0.5f);

        // 防护：半长不能为负
        obb.half_extents[0] = std::max(obb.half_extents[0], 1);
        obb.half_extents[1] = std::max(obb.half_extents[1], 1);

        return obb;
    }

    // 判断点是否在OBB内（快速剔除无效像素）
    bool contains_point(const Vec2i& p) const {
        Vec2i dir = p - center;
        // 投影到两个轴向，判断是否在半长范围内
        int proj_p = dir.dot(axes[0]);
        int proj_q = dir.dot(axes[1]);

        return std::abs(proj_p) <= half_extents[0] && std::abs(proj_q) <= half_extents[1];
    }

    // 获取OBB的像素遍历范围（最小/最大x/y）
    void get_bounds(int& min_x, int& max_x, int& min_y, int& max_y) const {
        // 计算OBB四个顶点
        Vec2i corner[4] = {
            center + axes[0] * half_extents[0] + axes[1] * half_extents[1],
            center + axes[0] * half_extents[0] - axes[1] * half_extents[1],
            center - axes[0] * half_extents[0] + axes[1] * half_extents[1],
            center - axes[0] * half_extents[0] - axes[1] * half_extents[1]
        };

        // 找四个顶点的极值（遍历范围）
        min_x = std::min({ corner[0].x, corner[1].x, corner[2].x, corner[3].x });
        max_x = std::max({ corner[0].x, corner[1].x, corner[2].x, corner[3].x });
        min_y = std::min({ corner[0].y, corner[1].y, corner[2].y, corner[3].y });
        max_y = std::max({ corner[0].y, corner[1].y, corner[2].y, corner[3].y });
    }
};