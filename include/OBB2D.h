#pragma once
#include "vector.h"
#include <algorithm>
#include <cmath>
#include <vector>

struct OBB2D {
    Vec2i center;          // OBB 中心（整数坐标，适配屏幕像素）
    Vec2i axes[2];         // 2 个正交单位轴向（自定义朝向，Vec2i 归一化后存储）
    int half_extents[2];   // 每个轴向的半长度（中心到边的距离，整数适配像素）

    // 默认构造：轴向为屏幕坐标系 X/Y 轴
    OBB2D() : center(Vec2i(0, 0)) {
        axes[0] = Vec2i(1, 0);  // X 轴
        axes[1] = Vec2i(0, 1);  // Y 轴
        half_extents[0] = 0;
        half_extents[1] = 0;
    }

    // 自定义构造：中心 + 轴向 + 半长
    // 注意：轴向会自动归一化为单位向量（浮点数计算后转整数，适配像素精度）
    OBB2D(const Vec2i& center_, const Vec2i axes_[2], const int half_extents_[2])
        : center(center_) {
        // 归一化轴向（浮点数计算，避免整数精度丢失）
        for (int i = 0; i < 2; i++) {
            float len = axes_[i].length();
            if (len < 1e-8) {
                axes[i] = (i == 0) ? Vec2i(1, 0) : Vec2i(0, 1); // 兜底默认轴向
            }
            else {
                axes[i].x = static_cast<int>(axes_[i].x / len);
                axes[i].y = static_cast<int>(axes_[i].y / len);
            }
            half_extents[i] = half_extents_[i];
        }
        // 确保轴向正交（修正浮点误差）
        if (std::abs(axes[0].dot(axes[1])) > 1e-4) {
            axes[1] = Vec2i(-axes[0].y, axes[0].x); // 垂直化
        }
    }

    // 从 2D 顶点集构造 OBB（简化版：基于 AABB 转换，适配屏幕像素）
    static OBB2D from_vertices(const Vec2i* vertices, int count) {
        OBB2D obb;
        if (count == 0) return obb;

        // 第一步：计算 AABB 包围盒（像素级最小/最大坐标）
        int min_x = vertices[0].x, max_x = vertices[0].x;
        int min_y = vertices[0].y, max_y = vertices[0].y;

        for (int i = 1; i < count; i++) {
            min_x = std::min(min_x, vertices[i].x);
            max_x = std::max(max_x, vertices[i].x);
            min_y = std::min(min_y, vertices[i].y);
            max_y = std::max(max_y, vertices[i].y);
        }

        // 设置 OBB 中心（整数像素）
        obb.center = Vec2i((min_x + max_x) / 2, (min_y + max_y) / 2);
        // 轴向为屏幕 X/Y 轴
        obb.axes[0] = Vec2i(1, 0);
        obb.axes[1] = Vec2i(0, 1);
        // 设置半长（整数像素）
        obb.half_extents[0] = (max_x - min_x) / 2;
        obb.half_extents[1] = (max_y - min_y) / 2;

        return obb;
    }

    // 判断 2D 点（像素坐标）是否在 OBB 内
    bool contains_point(const Vec2i& point) const {
        Vec2i dir = point - center;
        // 检查每个轴向的投影是否在半长范围内
        for (int i = 0; i < 2; i++) {
            int proj = dir.dot(axes[i]);
            if (std::abs(proj) > half_extents[i]) {
                return false;
            }
        }
        return true;
    }

    // 2D OBB 相交检测（分离轴定理 SAT）
    // 仅检测 2 个自身轴向 + 1 个对方轴向 → 共 4 个潜在分离轴
    bool intersects(const OBB2D& other) const {
        // 待检测的分离轴：自身2个轴向 + 对方2个轴向（共4个）
        Vec2i test_axes[4];
        test_axes[0] = axes[0];
        test_axes[1] = axes[1];
        test_axes[2] = other.axes[0];
        test_axes[3] = other.axes[1];

        for (int i = 0; i < 4; i++) {
            Vec2i axis = test_axes[i];
            float len = axis.length();
            if (len < 1e-8) continue; // 跳过零向量

            // 归一化轴（浮点数计算投影）
            Vec2i axis_norm(
                static_cast<int>(axis.x / len),
                static_cast<int>(axis.y / len)
            );

            // 计算自身 OBB 在轴上的投影区间
            int self_proj_center = center.dot(axis_norm);
            int self_extent = std::abs(axes[0].dot(axis_norm)) * half_extents[0] +
                std::abs(axes[1].dot(axis_norm)) * half_extents[1];
            int self_min = self_proj_center - self_extent;
            int self_max = self_proj_center + self_extent;

            // 计算对方 OBB 在轴上的投影区间
            int other_proj_center = other.center.dot(axis_norm);
            int other_extent = std::abs(other.axes[0].dot(axis_norm)) * other.half_extents[0] +
                std::abs(other.axes[1].dot(axis_norm)) * other.half_extents[1];
            int other_min = other_proj_center - other_extent;
            int other_max = other_proj_center + other_extent;

            // 投影分离 → OBB 不相交
            if (self_max < other_min || other_max < self_min) {
                return false;
            }
        }
        return true; // 所有轴都不分离 → 相交
    }

    // 扩展：将 3D 顶点投影到 2D 屏幕后构造 OBB
    static OBB2D from_3d_verts_project(const Vec3f* verts_3d, int count, int width, int height) {
        std::vector<Vec2i> verts_2d(count);
        // 投影 3D 顶点到 2D 屏幕（复用你的 project 函数逻辑）
        for (int i = 0; i < count; i++) {
            verts_2d[i].x = static_cast<int>((verts_3d[i].x + 1.0f) * width / 2.0f);
            verts_2d[i].y = static_cast<int>((verts_3d[i].y + 1.0f) * height / 2.0f);
        }
        return from_vertices(verts_2d.data(), count);
    }
};
