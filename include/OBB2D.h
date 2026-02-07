#pragma once

#include <vector>
#include "../include/vector.h"
#include <algorithm>

struct OBB2D {
    Vec2f center;          // OBB包围盒中心坐标
	Vec2f axes[2];         // OBB的两个正交轴
    float halfExtents[2];  // 半长/半宽

    OBB2D();

	OBB2D(const Vec2f& center, float width, float height, float rotationRad);

	OBB2D(const std::vector<Vec2f>& points);

	std::vector<Vec2f> getCorners() const;

	bool containsPoint(const Vec2f& point) const;

	bool Intersects(const OBB2D& other) const;

	void MoveTo(const Vec2f& newCenter);

	void Rotate(float angleRad);
};