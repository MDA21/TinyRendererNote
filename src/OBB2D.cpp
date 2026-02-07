#include "../include/OBB2D.h"
#include <limits>
#include <algorithm>

// 计算点集的OBB包围盒（PCA核心流程）
OBB OBB::compute(const std::vector<Vec2f>& points) {
    OBB obb;
    size_t pointCount = points.size();

    // 边界情况处理：空点集或单点
    if (pointCount == 0) return obb;
    if (pointCount == 1) {
        obb.center = points[0];
        obb.halfExtents[0] = obb.halfExtents[1] = Vec2f(0, 0);
        return obb;
    }

    // 步骤1：计算点集均值（中心化预处理）
    float muX = 0.0f, muY = 0.0f;
    for (const auto& p : points) {
        muX += p.x;
        muY += p.y;
    }
    muX /= pointCount;
    muY /= pointCount;

    // 步骤2：构建协方差矩阵
    float covXX = 0.0f, covXY = 0.0f, covYY = 0.0f;
    for (const auto& p : points) {
        float dx = p.x - muX;
        float dy = p.y - muY;
        covXX += dx * dx;
        covXY += dx * dy;
        covYY += dy * dy;
    }
    covXX /= pointCount;  // 方差X（除以样本数，不影响特征向量方向）
    covXY /= pointCount;  // 协方差XY
    covYY /= pointCount;  // 方差Y
    Mat2f covMat(covXX, covXY, covXY, covYY);

    // 步骤3：特征值分解（得到OBB的主方向）
    float eigenvalues[2];
    Mat2f eigenVectors;
    covMat.EVD(eigenvalues, eigenVectors);  // 复用你提供的Mat2f::EVD方法
    Vec2f e1(eigenVectors.m[0][0], eigenVectors.m[1][0]);  // 主方向（最大特征值）
    Vec2f e2(eigenVectors.m[0][1], eigenVectors.m[1][1]);  // 次方向（最小特征值）

    // 步骤4：将中心化点投影到新基，求UV空间AABB
    float umin = std::numeric_limits<float>::max();
    float umax = std::numeric_limits<float>::lowest();
    float vmin = std::numeric_limits<float>::max();
    float vmax = std::numeric_limits<float>::lowest();
    for (const auto& p : points) {
        Vec2f cp(p.x - muX, p.y - muY);  // 中心化点
        float u = cp.dot(e1);            // 投影到e1（U坐标）
        float v = cp.dot(e2);            // 投影到e2（V坐标）
        umin = std::min(umin, u);
        umax = std::max(umax, u);
        vmin = std::min(vmin, v);
        vmax = std::max(vmax, v);
    }

    // 步骤5：计算OBB的中心和半轴（转换回原坐标）
    float centerU = (umin + umax) * 0.5f;
    float centerV = (vmin + vmax) * 0.5f;
    obb.center.x = muX + centerU * e1.x + centerV * e2.x;
    obb.center.y = muY + centerU * e1.y + centerV * e2.y;
    obb.halfExtents[0] = e1 * ((umax - umin) * 0.5f);  // 主半轴
    obb.halfExtents[1] = e2 * ((vmax - vmin) * 0.5f);  // 次半轴

    return obb;
}

// 分离轴定理（SAT）实现OBB碰撞检测
bool OBB::checkCollision(const OBB& other) const {
    // 所有可能的分离轴：两个OBB的4个半轴方向
    const Vec2f axes[] = {
        halfExtents[0], halfExtents[1],
        other.halfExtents[0], other.halfExtents[1]
    };

    for (const auto& axis : axes) {
        if (axis.length_sq() < 1e-12f) continue;  // 跳过零向量轴

        // 计算当前OBB在轴上的投影区间
        float minA = std::numeric_limits<float>::max();
        float maxA = std::numeric_limits<float>::lowest();
        Vec2f verticesA[] = {
            center + halfExtents[0] + halfExtents[1],
            center + halfExtents[0] - halfExtents[1],
            center - halfExtents[0] + halfExtents[1],
            center - halfExtents[0] - halfExtents[1]
        };
        for (const auto& v : verticesA) {
            float proj = v.dot(axis);
            minA = std::min(minA, proj);
            maxA = std::max(maxA, proj);
        }

        // 计算另一个OBB在轴上的投影区间
        float minB = std::numeric_limits<float>::max();
        float maxB = std::numeric_limits<float>::lowest();
        Vec2f verticesB[] = {
            other.center + other.halfExtents[0] + other.halfExtents[1],
            other.center + other.halfExtents[0] - other.halfExtents[1],
            other.center - other.halfExtents[0] + other.halfExtents[1],
            other.center - other.halfExtents[0] - other.halfExtents[1]
        };
        for (const auto& v : verticesB) {
            float proj = v.dot(axis);
            minB = std::min(minB, proj);
            maxB = std::max(maxB, proj);
        }

        // 投影无重叠则无碰撞
        if (maxA < minB - 1e-8f || maxB < minA - 1e-8f) {
            return false;
        }
    }

    return true;  // 所有轴投影都重叠，存在碰撞
}