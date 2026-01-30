#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <set>
#include "../include/model.h"
#include "../include/tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

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

int project(float pos, int worh) {
	int screen_pos = static_cast<int>((pos + 1.0f) * worh / 2.0f);
	return screen_pos;
}
int main(int argc, char** argv) {
	

    constexpr int width  = 800;
    constexpr int height = 800;
	const int LOOP_TIMES = 1000;
    TGAImage framebuffer(width, height, TGAImage::RGB);

	//这里后续可以改成从命令行参数传入模型路径
    Model model("F:/VSproject/TinyRenderer/obj/diablo3_pose/diablo3_pose.obj");

	int num_faces = model.nfaces();
	int num_verts = model.nverts();
	std::set<std::pair<int, int>> edges;
	auto start_time = std::chrono::steady_clock::now();

	for (int k = 0; k < LOOP_TIMES; k++) {
		for (int i = 0; i < num_faces; i++) {
			for (int j = 0; j < 3; j++) {
				int v0_idx = model.vert_idx(i, j % 3);
				int v1_idx = model.vert_idx(i, (j + 1) % 3);
				if (v0_idx > v1_idx) std::swap(v0_idx, v1_idx);
				std::pair<int, int> edge = { v0_idx, v1_idx };

				if (edges.find(edge) == edges.end()) {

					edges.insert(edge);
					Vec3f v0 = model.vert(v0_idx);
					Vec3f v1 = model.vert(v1_idx);
					int screen_x0 = project(v0.x, width);
					int screen_y0 = project(v0.y, height);
					int screen_x1 = project(v1.x, width);
					int screen_y1 = project(v1.y, height);
					draw_line(screen_x0, screen_y0, screen_x1, screen_y1, framebuffer, red);
				}
			}
		}
	}
    
    framebuffer.write_tga_file("framebuffer.tga");

	auto end_time = std::chrono::steady_clock::now();
	auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	auto duration_s = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
	std::cout << "程序运行完成！" << std::endl;
	std::cout << "总运行时间：" << duration_ms << " 毫秒" << std::endl;
	std::cout << "总运行时间：" << duration_s << " 秒" << std::endl;

    return 0;
}

