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
#include "../include/obb2d.h"

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
	
	int bboxmin_x = std::min({ ax, bx, cx });
	int bboxmax_x = std::max({ ax, bx, cx });
	int bboxmin_y = std::min({ ay, by, cy });
	int bboxmax_y = std::max({ ay, by, cy });
	double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
	if (total_area < 1)return;


	// 策略：只有当 AABB 的面积远大于三角形面积时，才值得花算力去构建 OBB
	// 否则，构建 OBB 的开销比直接算重心坐标还要大
	double aabb_area = (bboxmax_x - bboxmin_x) * (bboxmax_y - bboxmin_y);

	bool use_obb_culling = false;

	OBB2D triOBB;

	//如果 AABB 面积是三角形的 3 倍以上，说明三角形很细长/倾斜，启用 OBB 剔除
	if ((aabb_area / total_area) > 3.0) {
		std::vector<Vec2f> pts;
		pts.reserve(3);
		pts.emplace_back((float)ax, (float)ay);
		pts.emplace_back((float)bx, (float)by);
		pts.emplace_back((float)cx, (float)cy);

		// 构建 OBB
		triOBB = OBB2D(pts);

		// 【关键】为了防止浮点误差导致边缘像素丢失，稍微扩大一点 OBB
		// 这是一个工程技巧，给半长增加 0.5 - 1.0 的容差
		triOBB.halfExtents[0] += 0.5f;
		triOBB.halfExtents[1] += 0.5f;

		use_obb_culling = true;
	}
#pragma omp parallel for
	for(int x = bboxmax_x; x >= bboxmin_x; x--) {
		for(int y = bboxmax_y; y >= bboxmin_y; y--) {

			if (use_obb_culling) {
				// 使用像素中心点 (x + 0.5, y + 0.5) 进行测试精度更高
				Vec2f pixelCenter((float)x + 0.5f, (float)y + 0.5f);

				if (!triOBB.containsPoint(pixelCenter)) {
					continue;
				}
			}

			double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
			if (alpha < 0)continue;

			double beta = signed_triangle_area(ax, ay, x, y, cx, cy) / total_area;
			if (beta < 0)continue;

			double gamma = signed_triangle_area(ax, ay, bx, by, x, y) / total_area;
			if (gamma < 0)continue;

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

