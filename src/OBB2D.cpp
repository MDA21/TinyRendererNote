#include "../include/OBB2D.h"
#include "../include/vector.h"
#include <algorithm>
#include <limits>
#include <cmath>

// ==========================================
// 构造方法
// ==========================================

OBB2D::OBB2D() {
	center = Vec2f(0.0f, 0.0f);
	axes[0] = Vec2f(1.0f, 0.0f);
	axes[1] = Vec2f(0.0f, 1.0f);
	halfExtents[0] = 1.0f;
	halfExtents[1] = 1.0f;
}

OBB2D::OBB2D(const Vec2f& center, float width, float height, float rotationRad) {
	this->center = center;
	halfExtents[0] = width * 0.5f;
	halfExtents[1] = height * 0.5f;

	float c = std::cos(rotationRad);
	float s = std::sin(rotationRad);

	axes[0] = Vec2f(c, s);   // x轴
	axes[1] = Vec2f(-s, c);  // y轴 (垂直于x轴)
}

OBB2D::OBB2D(const std::vector<Vec2f>& points) {
	if(points.empty()) {
		OBB2D();
	}
	//重心
	Vec2f mean(0.0f, 0.0f);
	for(const auto& p : points) {
		mean += p;
	}
	mean /= static_cast<float>(points.size());

	//构建协方差矩阵
	float cxx = 0.0f, cxy = 0.0f, cyy = 0.0f;
	for(const auto& p : points) {
		Vec2f d = p - mean;
		cxx += d.x * d.x;
		cxy += d.x * d.y;
		cyy += d.y * d.y;
	}

	float invN = 1.0 / points.size();
	cxx *= invN;
	cxy *= invN;
	cyy *= invN;

	Mat2f covariance(cxx, cxy, cxy, cyy);

	//SVD分解协方差矩阵，得到特征值和特征向量
	float eigenValues[2];
	Mat2f eigenVectors;
	covariance.SymEigens(eigenValues, eigenVectors);

	//基向量，已经归一化了
	axes[0] = Vec2f(eigenVectors.m[0][0], eigenVectors.m[1][0]); //主特征向量,也就是主轴
	axes[1] = Vec2f(eigenVectors.m[0][1], eigenVectors.m[1][1]); //次特征向量，也就是次轴

	//将所有点投影到主轴和次轴上，找到最大最小投影值，计算半长半宽
	float min0 = std::numeric_limits<float>::max();
	float max0 = std::numeric_limits<float>::lowest();
	float min1 = std::numeric_limits<float>::max();
	float max1 = std::numeric_limits<float>::lowest();

	for(const auto& p : points) {
		//投影公式: p_local = (p - origin) dot axis
	    //这里可以直接投影 p dot axis，后面再算偏移
		float proj0 = p.dot(axes[0]);
		float proj1 = p.dot(axes[1]);
		min0 = std::min(min0, proj0);
		max0 = std::max(max0, proj0);
		min1 = std::min(min1, proj1);
		max1 = std::max(max1, proj1);
	}

	//半长半宽
	halfExtents[0] = (max0 - min0) * 0.5f;
	halfExtents[1] = (max1 - min1) * 0.5f;

	//几何中心还原回世界坐标
	float mid0 = (min0 + max0) * 0.5f;
	float mid1 = (min1 + max1) * 0.5f;

	center = axes[0] * mid0 + axes[1] * mid1;
}

// ==========================================
// 功能方法
// ==========================================

std::vector<Vec2f> OBB2D::getCorners() const {
	std::vector<Vec2f> corners(4);
	Vec2f ext0 = axes[0] * halfExtents[0];
	Vec2f ext1 = axes[1] * halfExtents[1];

	// P = C + (dir_x * axis0) + (dir_y * axis1)，其中 dir_x 和 dir_y 可以是 -1 或 +1，分别对应四个角的组合
	corners[0] = center - ext0 - ext1; // 左下
	corners[1] = center + ext0 - ext1; // 右下
	corners[2] = center + ext0 + ext1; // 右上
	corners[3] = center - ext0 + ext1; // 左上
	return corners;
}

bool OBB2D::containsPoint(const Vec2f& point) const {
	// 将点投影到OBB的局部坐标系中
	Vec2f d = point - center;
	
	//投影到两个轴上
	float proj0 = d.dot(axes[0]);
	float proj1 = d.dot(axes[1]);

	return std::abs(proj0) <= halfExtents[0] && std::abs(proj1) <= halfExtents[1];
}

bool OBB2D::Intersects(const OBB2D& other) const {
	// 使用分离轴定理 (Separating Axis Theorem) 来判断两个OBB是否相交
	// SAT 原理：如果两个凸多边形不相交，必然存在一条“分离轴”，
    // 使得两个多边形在该轴上的投影不重叠。
    // 对于 OBB，只需要检测 4 个轴：
    // A 的 2 个轴，B 的 2 个轴。
	// 需要测试的轴: this.axes[0], this.axes[1], other.axes[0], other.axes[1]
	Vec2f testAxes[4] = { this->axes[0], this->axes[1], other.axes[0], other.axes[1] };

	for(int i = 0; i < 4; i++) {
		Vec2f axis = testAxes[i];
		
		// 轴可能不是归一化的吗？我们的算法保证 axes 是归一化的。
		// 但为了通用性，如果 axis 长度不为 1，下面的投影计算需要除以长度。
		// 这里假设 axes 都是单位向量。

		// 计算 A (this) 在轴上的投影半径
		// 半径 = |axis . axis0| * e0 + |axis . axis1| * e1
		float rA = std::abs(axis.dot(this->axes[0])) * this->halfExtents[0] + 
			std::abs(axis.dot(this->axes[1])) * this->halfExtents[1];

		// 计算 B (other) 在轴上的投影半径
		float rB = std::abs(axis.dot(other.axes[0])) * other.halfExtents[0] + 
			std::abs(axis.dot(other.axes[1])) * other.halfExtents[1];

		// 计算两个中心点距离在轴上的投影
		float centerDist = std::abs((other.center - this->center).dot(axis));

		// 如果中心距离大于半径之和，说明在这个轴上有分离，两个OBB不相交
		if(centerDist > rA + rB) {
			return false; // 找到一个分离轴，提前返回
		}
	}
	return true; // 所有轴都没有分离，两个OBB相交
}

void OBB2D::MoveTo(const Vec2f& newCenter) {
	center = newCenter;
}

void OBB2D::Rotate(float angle_radian) {
	float c = std::cos(angle_radian);
	float s = std::sin(angle_radian);
	axes[0] = Vec2f(c, s);
	axes[1] = Vec2f(-s, c);
}