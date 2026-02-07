#pragma once
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream> // 用于简单的打印调试

// ==========================================
// Vec2f: 2D 浮点向量
// ==========================================
struct Vec2f {
    float x, y;

    Vec2f(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}

    Vec2f operator+(const Vec2f& other) const { return Vec2f(x + other.x, y + other.y); }

    Vec2f operator-(const Vec2f& other) const { return Vec2f(x - other.x, y - other.y); }

    Vec2f operator*(float s) const { return Vec2f(x * s, y * s); }

    Vec2f operator/(float s) const {
        float inv = 1.0f / (s != 0.0f ? s : 1.0f); 
        return Vec2f(x * inv, y * inv);
    }

	Vec2f& operator=(const Vec2f& other) { x = other.x; y = other.y; return *this; }

    Vec2f operator-() const { return Vec2f(-x, -y); }

    Vec2f& operator+=(const Vec2f& other) { x += other.x; y += other.y; return *this; }

    Vec2f& operator-=(const Vec2f& other) { x -= other.x; y -= other.y; return *this; }

    Vec2f& operator*=(float s) { x *= s; y *= s; return *this; }

    Vec2f& operator/=(float s) {
        float inv = 1.0f / (s != 0.0f ? s : 1.0f); 
        x *= inv; y *= inv; return *this;
	}

    //点乘
    float dot(const Vec2f& other) const { return x * other.x + y * other.y; }

    //叉乘 (2D下返回标量，代表Z轴分量)
    float cross(const Vec2f& other) const { return x * other.y - y * other.x; }

    //模长平方
    float length_sq() const { return x * x + y * y; }

    //模长
    float length() const { return std::sqrt(length_sq()); }

    //归一化
    Vec2f normalize() const {
        float lenSq = length_sq();
        if (lenSq < 1e-8f) return Vec2f(0, 0);
        float invLen = 1.0f / std::sqrt(lenSq);
        return Vec2f(x * invLen, y * invLen);
    }

    //两点距离平方
    float distance_sq(const Vec2f& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        return dx * dx + dy * dy;
    }

    //两点距离
    float distance(const Vec2f& other) const { return std::sqrt(distance_sq(other)); }

    //垂直向量 (逆时针旋转90度: -y, x)
    Vec2f perp() const { return Vec2f(-y, x); }


};




// ==========================================
// Vec2i: 2D 整数向量 
// ==========================================
struct Vec2i {
    int x, y;
    Vec2i(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}

    // 隐式转换为 Vec2f
    operator Vec2f() const { return Vec2f((float)x, (float)y); }

    Vec2i operator+(const Vec2i& other) const { return Vec2i(x + other.x, y + other.y); }
    Vec2i operator-(const Vec2i& other) const { return Vec2i(x - other.x, y - other.y); }
    Vec2i operator*(int s) const { return Vec2i(x * s, y * s); }

    bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2i& other) const { return !(*this == other); }
};


// ==========================================
// Vec3f: 3D 浮点向量
// ==========================================
struct Vec3f {
    float x, y, z;
    Vec3f(float x_ = 0, float y_ = 0, float z_ = 0) : x(x_), y(y_), z(z_) {}

    Vec3f operator+(const Vec3f& o) const { return Vec3f(x + o.x, y + o.y, z + o.z); }
    Vec3f operator-(const Vec3f& o) const { return Vec3f(x - o.x, y - o.y, z - o.z); }
    Vec3f operator*(float s) const { return Vec3f(x * s, y * s, z * s); }

    float dot(const Vec3f& o) const { return x * o.x + y * o.y + z * o.z; }

    Vec3f cross(const Vec3f& o) const {
        return Vec3f(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }

    Vec3f normalize() const {
        float len = std::sqrt(x * x + y * y + z * z);
        return len < 1e-8f ? Vec3f(0, 0, 0) : Vec3f(x / len, y / len, z / len);
    }
};


// ==========================================
// Mat2f: 2x2 矩阵
// 存储方式: 行优先 (Row-Major)
// m[0][0] m[0][1]
// m[1][0] m[1][1]
// ==========================================
struct Mat2f {
    float m[2][2];

    // 默认单位矩阵
    Mat2f();
    Mat2f(float m00, float m01, float m10, float m11);

    //根据旋转角度(弧度)构建旋转矩阵
    static Mat2f Rotation(float angleRad);
    //根据两个基向量构建矩阵 (列向量)
    static Mat2f FromBasis(const Vec2f& x_axis, const Vec2f& y_axis);

    Mat2f operator+(const Mat2f& other) const;
    Mat2f operator*(float s) const;

    //矩阵乘向量
    Vec2f operator*(const Vec2f& v) const;

    //矩阵乘矩阵
    Mat2f operator*(const Mat2f& other) const;

    //转置
    Mat2f transpose() const;

    //行列式
    float determinant() const;

    /**
     * @brief 对称矩阵特征值分解 (Symmetric Eigen Value Decomposition)
     * 专用于 PCA / OBB 计算。
     *
     * @param outEigenValues [输出] 两个特征值，eigenvalues[0] 对应 col[0]
     * @param outEigenVectors [输出] 特征向量矩阵。
     *        第一列(col 0)是主特征向量(对应eigenvalues[0])，
     *        第二列(col 1)是次特征向量。
     *        注意：特征向量已归一化。
     */
    void SymEigens(float outEigenValues[2], Mat2f& outEigenVectors) const;
};