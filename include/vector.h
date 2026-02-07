#pragma once
#include <cmath>

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
    Vec2f to_float() const { return Vec2f((float)x, (float)y); }


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