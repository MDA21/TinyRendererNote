#pragma once
#include <cmath>
#include <limits>
#include <algorithm>


struct Vec2f {
    float x, y;
    Vec2f(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}

    // 向量点乘
    float dot(const Vec2f& other) const {
        return x * other.x + y * other.y;
    }

    // 向量叉乘（2D下返回标量，等于z轴分量）
    float cross(const Vec2f& other) const {
        return x * other.y - y * other.x;
    }

    // 向量归一化
    Vec2f normalize() const {
        float len = std::sqrt(x * x + y * y);
        return len < 1e-8 ? Vec2f(0, 0) : Vec2f(x / len, y / len);
    }

    // 向量减法
    Vec2f operator-(const Vec2f& other) const {
        return Vec2f(x - other.x, y - other.y);
    }

    Vec2f operator+(const Vec2f& other) const {
        return Vec2f(x + other.x, y + other.y);
    }

    // 向量数乘
    Vec2f operator*(float s) const {
        return Vec2f(x * s, y * s);
    }
};

struct Vec2i
{
    int x, y;
    Vec2i(int x = 0, int y = 0) : x(x), y(y) {}
    operator Vec2f() const {
        return Vec2f(static_cast<float>(x), static_cast<float>(y));
    }


    Vec2i operator+(const Vec2i& other) const {
        return Vec2i(x + other.x, y + other.y);
    }

    Vec2i operator-(const Vec2i& other) const {
        return Vec2i(x - other.x, y - other.y);
    }

    Vec2i operator*(int scalar) const {
        return Vec2i(x * scalar, y * scalar);
    }

    Vec2i operator/(int scalar) const {
        if (scalar == 0) return Vec2i(x, y);
        return Vec2i(x / scalar, y / scalar);
    }

    int dot(const Vec2i& other) const {
        return x * other.x + y * other.y;
    }

    int cross(const Vec2i& other) const {
        return x * other.y - y * other.x;
    }

    float length_sq() const {
        return x * x + y * y;
    }
    float length() const {
        return std::sqrt(length_sq());
    }
};

struct Vec3f
{
    float x, y, z;
    Vec3f(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3f operator+(const Vec3f& other) const {
        return Vec3f(x + other.x, y + other.y, z + other.z);
    }
    Vec3f operator-(const Vec3f& other) const {
        return Vec3f(x - other.x, y - other.y, z - other.z);
    }
    Vec3f operator*(float scalar) const {
        return Vec3f(x * scalar, y * scalar, z * scalar);
    }
    Vec3f operator/(float scalar) const {
        return Vec3f(x / scalar, y / scalar, z / scalar);
    }

    float dot(const Vec3f& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3f cross(const Vec3f& other) const {
        return Vec3f(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float length_sq() const {
        return x * x + y * y + z * z;
    }

    float length() const {
        return std::sqrt(length_sq());
    }

    Vec3f normalize() const {
        float len = length();
        if (len < 1e-8) return Vec3f(0, 0, 0);
        return Vec3f(x / len, y / len, z / len);
    }
};

struct Mat2f {
    float m[2][2];

    Mat2f() {
        m[0][0] = m[0][1] = m[1][0] = m[1][1] = 0.0f;
    }

    Mat2f(float m00, float m01, float m10, float m11) {
        m[0][0] = m00; m[0][1] = m01;
        m[1][0] = m10; m[1][1] = m11;
    }

    // 2x2矩阵特征值分解（解析解，无第三方库依赖）
    // 参数说明：
    // - eigenvalues[2]：输出特征值（降序排列）
    // - eigenvectors：输出特征向量矩阵（列向量存储，m[*][0] = 主特征向量，m[*][1] = 次特征向量）
    void EVD(float eigenvalues[2], Mat2f& eigenvectors) const {
        // 1. 计算矩阵迹（对角线和）、行列式
        float tr = m[0][0] + m[1][1];
        float det = m[0][0] * m[1][1] - m[0][1] * m[1][0];

        // 2. 计算特征值（数值稳定性处理，避免开方负数）
        float delta = tr * tr - 4 * det;
        delta = std::max(delta, 0.0f);
        float sqrt_delta = std::sqrt(delta);

        eigenvalues[0] = (tr + sqrt_delta) / 2.0f; // 主特征值（更大）
        eigenvalues[1] = (tr - sqrt_delta) / 2.0f; // 次特征值（更小）

        // 3. 计算特征向量（列向量，归一化）
        for (int i = 0; i < 2; ++i) {
            float lambda = eigenvalues[i];
            if (std::fabs(m[0][1]) > 1e-8f) { // 非对角矩阵
                float x = -m[0][1];
                float y = m[0][0] - lambda;
                float len = std::sqrt(x * x + y * y);
                eigenvectors.m[0][i] = x / len;
                eigenvectors.m[1][i] = y / len;
            }
            else { // 对角矩阵，特征向量为单位轴
                eigenvectors.m[0][i] = (i == 0) ? 1.0f : 0.0f;
                eigenvectors.m[1][i] = (i == 0) ? 0.0f : 1.0f;
            }
        }
    }
};