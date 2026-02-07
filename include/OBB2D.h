#include "vector.h"
#include <algorithm>
#include <cmath>
#include <limits>

struct OBB2D {
    Vec2f center;    // 改用浮点类型保存中心点（核心修正）
    Vec2f axis[2];   // 改用浮点类型保存OBB的轴（单位向量，核心修正）
    float extent[2]; // 改用浮点类型保存轴方向的半长（核心修正）

    // 从三角形构造OBB（核心修正）
    static OBB2D from_triangle(const Vec2i& v0, const Vec2i& v1, const Vec2i& v2) {
        OBB2D obb;
        std::vector<Vec2f> pts = { v0.to_float(), v1.to_float(), v2.to_float() };

        // 步骤1：计算三角形的质心（浮点精度）
        obb.center = (pts[0] + pts[1] + pts[2]) * (1.0f / 3.0f);

        // 步骤2：计算协方差矩阵 [a, b; b, c]
        float a = 0.0f, b = 0.0f, c = 0.0f;
        for (const auto& p : pts) {
            Vec2f delta = p - obb.center;
            a += delta.x * delta.x;
            b += delta.x * delta.y;
            c += delta.y * delta.y;
        }
        a /= 3.0f;
        b /= 3.0f;
        c /= 3.0f;

        // 步骤3：求解协方差矩阵的特征值和特征向量（OBB的主轴）
        // 特征方程：λ² - (a+c)λ + (ac - b²) = 0
        float trace = a + c;
        float det = a * c - b * b;
        float sqrt_det = sqrt(std::max(0.0f, det));

        // 两个特征值对应的特征向量（主轴方向）
        // 特征向量方向由 (b, λ - a) 确定
        float lambda1 = (trace + sqrt_det) / 2.0f;
        float lambda2 = (trace - sqrt_det) / 2.0f;

        // 主轴1（对应最大特征值，保证轴的正交性）
        Vec2f axis0, axis1;
        if (fabs(b) < 1e-8) { // 协方差矩阵对角化，轴沿x/y
            axis0 = Vec2f(1.0f, 0.0f);
            axis1 = Vec2f(0.0f, 1.0f);
        }
        else {
            axis0 = Vec2f(b, lambda1 - a).normalize();
            axis1 = Vec2f(-axis0.y, axis0.x); // 正交轴（逆时针旋转90度）
        }

        // 确保轴是单位向量（核心：保留旋转方向）
        obb.axis[0] = axis0.normalize();
        obb.axis[1] = axis1.normalize();

        // 步骤4：计算三角形顶点在两个轴上的投影范围，得到extent（半长）
        for (int i = 0; i < 2; i++) {
            float min_proj = std::numeric_limits<float>::max();
            float max_proj = std::numeric_limits<float>::lowest();
            for (const auto& p : pts) {
                Vec2f delta = p - obb.center;
                float proj = delta.dot(obb.axis[i]); // 浮点投影（无精度丢失）
                min_proj = std::min(min_proj, proj);
                max_proj = std::max(max_proj, proj);
            }
            obb.extent[i] = (max_proj - min_proj) / 2.0f; // 半长（浮点）
        }

        return obb;
    }

    // 判断点是否在OBB内（适配浮点类型）
    bool contains_point(const Vec2i& p) const {
        Vec2f p_float = p.to_float();
        Vec2f delta = p_float - center;

        // 投影到两个轴上，判断是否在extent范围内
        for (int i = 0; i < 2; i++) {
            float proj = delta.dot(axis[i]);
            if (fabs(proj) > extent[i] + 1e-8) { // 加小epsilon避免浮点误差
                return false;
            }
        }
        return true;
    }

    // 获取OBB的AABB边界（用于遍历像素，适配浮点转整型）
    void get_bounds(int& min_x, int& max_x, int& min_y, int& max_y) const {
        // 计算OBB的四个角点（浮点）
        std::vector<Vec2f> corners = {
            center + axis[0] * extent[0] + axis[1] * extent[1],
            center + axis[0] * extent[0] - axis[1] * extent[1],
            center - axis[0] * extent[0] + axis[1] * extent[1],
            center - axis[0] * extent[0] - axis[1] * extent[1]
        };

        // 遍历角点求AABB边界（转整型时取整）
        min_x = static_cast<int>(floor(corners[0].x));
        max_x = static_cast<int>(ceil(corners[0].x));
        min_y = static_cast<int>(floor(corners[0].y));
        max_y = static_cast<int>(ceil(corners[0].y));

        for (const auto& c : corners) {
            min_x = std::min(min_x, static_cast<int>(floor(c.x)));
            max_x = std::max(max_x, static_cast<int>(ceil(c.x)));
            min_y = std::min(min_y, static_cast<int>(floor(c.y)));
            max_y = std::max(max_y, static_cast<int>(ceil(c.y)));
        }
    }
};