#pragma once
#include <cmath>

struct Vec2i
{
	int x, y;
	Vec2i(int x = 0, int y = 0) : x(x), y(y) {}

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