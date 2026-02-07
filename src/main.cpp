#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "../include/model.h"
#include "../include/tgaimage.h"
#include "../include/OBB2D.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

struct Edge {
	int u, v;
	//重载 < 运算符，用于 sort
	bool operator<(const Edge& other) const {
		if (u != other.u) return u < other.u;
		return v < other.v;
	}
	//重载 == 运算符，用于判断重复
	bool operator==(const Edge& other) const {
		return u == other.u && v == other.v;
	}
};

void draw_line(int x0, int y0, int x1, int y1, TGAImage& framebuffer, TGAColor color) {
	//画线段的Bresenham算法实现
	bool steep = false;
	steep = (std::abs(x0 - x1) < std::abs(y0 - y1));
	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if(x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = std::abs(y1 - y0);
	int error = 0;
	int ystep = (y0 < y1) ? 1 : -1;
	int y = y0;

	for (int x = x0; x <= x1; x++) {
		if (steep) {
			framebuffer.set(y, x, color);
		} else {
			framebuffer.set(x, y, color);
		}

		error += dy;

		if (error * 2 >= dx) {
			y += ystep;
			error -= dx;
		}
	}
}

//worh: width or height
int project(float pos, int worh) {
	int screen_pos = static_cast<int>((pos + 1.0f) * worh / 2.0f);
	return screen_pos;
}

void loadModelOutline(Model model, TGAImage& framebuffer, int height, int width) {
	//加载模型轮廓并绘制到framebuffer上
	int num_faces = model.nfaces();
	int num_verts = model.nverts();
	std::vector<Edge> unique_edges;
	unique_edges.reserve(num_faces * 3);

	for (int i = 0; i < num_faces; i++) {
		for (int j = 0; j < 3; j++) {
			int v0_idx = model.vert_idx(i, j % 3);
			int v1_idx = model.vert_idx(i, (j + 1) % 3);
			if (v0_idx > v1_idx) std::swap(v0_idx, v1_idx);
			Edge edge = { v0_idx, v1_idx };
			unique_edges.push_back(edge);
		}
	}

	std::sort(unique_edges.begin(), unique_edges.end());
	auto last = std::unique(unique_edges.begin(), unique_edges.end());
	unique_edges.erase(last, unique_edges.end());


	std::vector<Vec2i> screen_coords(num_verts);



	for (int i = 0; i < num_verts; i++) {
		Vec3f v = model.vert(i);
		screen_coords[i].x = project(v.x, width);
		screen_coords[i].y = project(v.y, height);
	}


	for (const auto& edge : unique_edges) {


		Vec2i p0 = screen_coords[edge.u];
		Vec2i p1 = screen_coords[edge.v];

		// 简单的裁剪检查（可选，防止画出界崩溃）
		// if (p0.x < 0 || p0.x >= width || p0.y < 0 ...) continue;

		draw_line(p0.x, p0.y, p1.x, p1.y, framebuffer, red);
	}


	framebuffer.write_tga_file("framebuffer.tga");
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) {
	return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) {
	// 1. 构造三角形的OBB（对齐主轴，紧凑包围）
	Vec2i v0(ax, ay), v1(bx, by), v2(cx, cy);
	OBB2D tri_obb = OBB2D::from_triangle(v0, v1, v2);

	// 2. 获取OBB的像素遍历范围（比AABB小）
	int obb_min_x, obb_max_x, obb_min_y, obb_max_y;
	tri_obb.get_bounds(obb_min_x, obb_max_x, obb_min_y, obb_max_y);

	// 3. 计算三角形总面积（重心坐标判断用）
	float total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
	if (std::abs(total_area) < 1e-8) return;

	// 4. 遍历OBB范围内的像素（比AABB少）
	for (int x = obb_min_x; x <= obb_max_x; x++) {
		for (int y = obb_min_y; y <= obb_max_y; y++) {
			// 快速剔除OBB外的像素（第一层过滤）
			if (!tri_obb.contains_point(Vec2i(x, y))) {
				continue;
			}

			// 重心坐标判断是否在三角形内（第二层精准过滤）
			double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
			if (alpha < 0) continue;

			double beta = signed_triangle_area(ax, ay, x, y, cx, cy) / total_area;
			if (beta < 0) continue;

			double gamma = signed_triangle_area(ax, ay, bx, by, x, y) / total_area;
			if (gamma < 0) continue;

			// 点在三角形内（含边），填充像素
			
			framebuffer.set(x, y, color);
			
		}
	}
}



int main(int argc, char** argv) {
	

    constexpr int width  = 800;
    constexpr int height = 800;
    TGAImage framebuffer(width, height, TGAImage::RGB);
	const int LOOP_TIMES = 1000;
	

	//这里后续可以改成从命令行参数传入模型路径
    Model model("F:/VSproject/TinyRenderer/obj/diablo3_pose/diablo3_pose.obj");
	//loadModelOutline(model, framebuffer, height, width);
	auto start_time = std::chrono::steady_clock::now();

	for (int i = 0; i < LOOP_TIMES; i++) {

		for (int i = 0; i < model.nfaces(); i++) {
			Vec3f v0 = model.vert(model.vert_idx(i, 0));
			Vec3f v1 = model.vert(model.vert_idx(i, 1));
			Vec3f v2 = model.vert(model.vert_idx(i, 2));
			int ax = project(v0.x, width);
			int ay = project(v0.y, height);
			int bx = project(v1.x, width);
			int by = project(v1.y, height);
			int cx = project(v2.x, width);
			int cy = project(v2.y, height);
			TGAColor rnd;

			for (int c = 0; c < 3; c++) rnd[c] = std::rand() % 255;

			triangle(ax, ay, bx, by, cx, cy, framebuffer, rnd);
		}
	}


	framebuffer.write_tga_file("Triangle.tga");

	auto end_time = std::chrono::steady_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
	std::cout << "程序运行完成！" << std::endl;
	std::cout << "总运行时间：" << duration_ms << " 毫秒" << std::endl;
	std::cout << "总运行时间：" << duration_s << " 秒" << std::endl;
	

    return 0;
}

