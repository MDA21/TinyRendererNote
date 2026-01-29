#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
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
	for (int x = x0; x <= x1; x++) {
		float t = (x - x0) / static_cast<float>(x1 - x0);
		int y = std::round(y0 + (y1 - y0) * t);

		if (steep) {
			framebuffer.set(y, x, color);
		} else {
			framebuffer.set(x, y, color);
		}
	}
}

int main(int argc, char** argv) {
    constexpr int width  = 800;
    constexpr int height = 800;
    TGAImage framebuffer(width, height, TGAImage::RGB);

	//这里后续可以改成从命令行参数传入模型路径
    Model model("F:/VSproject/TinyRenderer/obj/diablo3_pose/diablo3_pose.obj");

	int num_faces = model.nfaces();

	for (int i = 0; i < num_faces; i++) {
        for (int j = 0; j < 3; j++) {
			Vec3f v0 = model.vert(i, j);
			Vec3f v1 = model.vert(i, (j + 1) % 3);
			int screen_x0 = static_cast<int>((v0.x + 1.0f) * width / 2.0f);
			int screen_y0 = static_cast<int>((v0.y + 1.0f) * height / 2.0f);
			int screen_x1 = static_cast<int>((v1.x + 1.0f) * width / 2.0f);
			int screen_y1 = static_cast<int>((v1.y + 1.0f) * height / 2.0f);
			draw_line(screen_x0, screen_y0, screen_x1, screen_y1, framebuffer, red);
        }
	}
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}

