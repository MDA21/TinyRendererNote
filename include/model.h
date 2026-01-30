#pragma once
#include <string>
#include <vector>
#include "vector.h"

class Model
{
public:
	Model(const std::string& filename);
	int nverts() const;		//返回顶点数量
	int nfaces() const;		//返回面数量
	Vec3f vert(const int i) const;		//返回第i个顶点坐标
	Vec3f vert(const int iface, const int nthvert) const;		//返回第iface个面第nthvert个顶点坐标
	int vert_idx(const int iface, const int jvert) const;
	std::vector<std::vector<int>> faces;
private:
	std::vector<Vec3f> verts;
	
};

void Log(const std::string& message);