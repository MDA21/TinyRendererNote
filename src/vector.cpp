#include "../include/vector.h"

// ==========================================
// Mat2f 实现
// ==========================================

Mat2f::Mat2f() {
    m[0][0] = 1.0f; m[0][1] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 1.0f;
}

Mat2f::Mat2f(float m00, float m01, float m10, float m11) {
    m[0][0] = m00; m[0][1] = m01;
    m[1][0] = m10; m[1][1] = m11;
}

Mat2f Mat2f::Rotation(float angleRad) {
    float c = std::cos(angleRad);
    float s = std::sin(angleRad);
    // 2D 旋转矩阵
    // [ cos -sin ]
    // [ sin  cos ]
    return Mat2f(c, -s, s, c);
}

Mat2f Mat2f::FromBasis(const Vec2f& x_axis, const Vec2f& y_axis) {
    // 将基向量作为列填充
    return Mat2f(x_axis.x, y_axis.x,
        x_axis.y, y_axis.y);
}

Mat2f Mat2f::operator+(const Mat2f& other) const {
    return Mat2f(m[0][0] + other.m[0][0], m[0][1] + other.m[0][1],
        m[1][0] + other.m[1][0], m[1][1] + other.m[1][1]);
}

Mat2f Mat2f::operator*(float s) const {
    return Mat2f(m[0][0] * s, m[0][1] * s,
        m[1][0] * s, m[1][1] * s);
}

// 矩阵 x 向量
// [ a b ] [ x ]   [ ax + by ]
// [ c d ] [ y ] = [ cx + dy ]
Vec2f Mat2f::operator*(const Vec2f& v) const {
    return Vec2f(
        m[0][0] * v.x + m[0][1] * v.y,
        m[1][0] * v.x + m[1][1] * v.y
    );
}

// 矩阵 x 矩阵
Mat2f Mat2f::operator*(const Mat2f& o) const {
    return Mat2f(
        m[0][0] * o.m[0][0] + m[0][1] * o.m[1][0], m[0][0] * o.m[0][1] + m[0][1] * o.m[1][1],
        m[1][0] * o.m[0][0] + m[1][1] * o.m[1][0], m[1][0] * o.m[0][1] + m[1][1] * o.m[1][1]
    );
}

Mat2f Mat2f::transpose() const {
    return Mat2f(m[0][0], m[1][0],
        m[0][1], m[1][1]);
}

float Mat2f::determinant() const {
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

void Mat2f::SymEigens(float outEigenValues[2], Mat2f& outEigenVectors) const {
    // 针对协方差矩阵 (实对称矩阵) 的优化算法
    // 矩阵形式:
    // [ a  b ]
    // [ b  d ]
    float a = m[0][0];
    float b = m[0][1]; // 必须等于 m[1][0]
    float d = m[1][1];

    // 1. 如果 b 非常小，说明已经是做对角阵（已经对齐轴了）
    if (std::abs(b) < 1e-8f) {
        outEigenValues[0] = a;
        outEigenValues[1] = d;
        outEigenVectors = Mat2f(1, 0, 0, 1); // 已经是单位阵
        return;
    }

    // 2. 求解特征值的迹和行列式
    // 特征方程: lambda^2 - tr*lambda + det = 0
    float trace = a + d;
    float det = a * d - b * b;

    // 判别式 delta = (a+d)^2 - 4(ad-b^2) = (a-d)^2 + 4b^2
    // 对于对称矩阵，delta 恒非负
    float gap = a - d;
    float delta = std::sqrt(gap * gap + 4.0f * b * b);

    float l1 = (trace + delta) * 0.5f;
    float l2 = (trace - delta) * 0.5f;

    outEigenValues[0] = l1;
    outEigenValues[1] = l2;

    // 3. 求解特征向量 (旋转角度法)
    // 对于 2D 对称矩阵，可以使用 atan2 直接求出主轴角度
    // theta = 0.5 * atan2(2b, a - d)
    float theta = 0.5f * std::atan2(2.0f * b, a - d);

    float c = std::cos(theta);
    float s = std::sin(theta);

    // 主特征向量 (对应较大的特征值)
    // v1 = (cos, sin)
    outEigenVectors.m[0][0] = c;
    outEigenVectors.m[1][0] = s;

    // 次特征向量 (垂直于主向量)
    // v2 = (-sin, cos)
    outEigenVectors.m[0][1] = -s;
    outEigenVectors.m[1][1] = c;
}