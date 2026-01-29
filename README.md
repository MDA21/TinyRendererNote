# TinyRenderer个人学习笔记



## Bresenham画线

这部分的目标是搓一个能画`.obj`轮廓的程序

教程中提供了`tgaimage.h`和`tgaimage.cpp`，其中包含了对`.tga`格式最基本的处理函数，剩下的都是自己搓。

### 第一次尝试

#### 文件结构

```
tinyrenderer/
├── include/           # 所有头文件目录
│   ├── vector.h
│   ├── model.h
│   └── tgaimage.h
└── src/               # 所有源文件目录
    ├── model.cpp
    ├── tgaimage.cpp
    └── main.cpp

```

#### vector.h

我们需要一种数据结构来存储顶点（vertex）的xyz坐标，由于不能用`Eigen`库，我们自己定义一个简单的结构就行，这就是`vector.h`。

```c++
struct Vec2i
{
	int x, y;
	Vec2i(int x = 0, int y = 0) : x(x), y(y) {}
};

struct Vec3f
{
	float x, y, z;
	Vec3f(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};
```

分别包含一个二维整数向量和一个三维浮点向量的定义。

tips: 标准库里的std::vector是动态数组。

#### model.h

这个头文件包含了Model类的定义和方法，其中`filename`就是文件的路径。

```c++
#include <string>
#include <vector>
#include "vector.h"

class Model
{
public:
	Model(const std::string& filename);		//加载模型
	int nverts() const;		//返回顶点数量
	int nfaces() const;		//返回面数量
	Vec3f vert(const int i) const;		//返回第i个顶点坐标
	Vec3f vert(const int iface, const int nthvert) const;	
	//返回第iface个面第nthvert个顶点坐标
    
private:
	std::vector<Vec3f> verts;
	std::vector<std::vector<int>> faces;
};

void Log(const std::string& message);	
```

#### model.cpp

主要的函数是`Model::Model(const std::string& filename)`，用于加载模型

教程提供了一个[wavefront obj](https://en.wikipedia.org/wiki/Wavefront_.obj_file)文件，我们需要先大致了解一下这种格式，这种格式是纯文本的，可以直接用vscode打开。

一个`.obj`文件通常存储了两大类信息：**数据点** 和 **拓扑结构**

数据点部分通常位于文件的前半部分，定义了模型所有的原材料

```
v 0.11526 0.700717 0.0677257
...

vt  0.067 0.290 0.173
...

vn  0.447 0.893 -0.051
..
```

v - 几何顶点

vt - 纹理坐标

vn - 顶点法线

拓扑部分部分是 f (Face)，它不存储坐标，只存储索引。它告诉计算机：把第几个点、第几个纹理、第几个法线连起来，构成一个面，`f v/vt/vn v/vt/vn  v/vt/vn`。

```
f 680/701/680 679/700/679 684/738/684
```

大多数现代 GPU 只能画三角形，如果你读到了四边形，通常需要在代码里把它拆分成两个三角形。本次提供的暗黑破坏神的模型都是三角形，但是如果去blender直接导出一个立方体的话，你就会发现 f 行每行有四组，不过我在第一次实现中没有管四边形。

剩下的基本上就是标准库的使用了，c with stl这一块。在第一部分的内容中，我们不需要管法线和纹理，也就是说，只需要读取和识别 **v 和 f** ，其余部分可以都丢掉。

需要注意的是，**f 的索引从 1 开始**，所以把面存入动态数组时需要把索引减1。

```c++
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "../include/model.h"

void Log(const std::string& message) {
	std::cout << message << std::endl;
}

Model::Model(const std::string& filename) {
	std::ifstream in;
	in.open(filename);

	if (!in.is_open()) {
		Log("Cannot open file: " + filename);
		return;
	}

	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "v") {
			float x, y, z;
			iss >> x >> y >> z;
			verts.push_back(Vec3f(x, y, z));
		}
		else if (type == "f") {
			//blender的导出格式是 f  v/vt/vn..
			std::vector<int> face_indices;
			int idx;
			char trash; //用来吃掉斜杠

			while (iss >> idx) {
				idx--;
				face_indices.push_back(idx);

				if(iss.peek() == '/') {
					iss >> trash;
					if (iss.peek() != '/') {
						//处理纹理vt
						int vt;
						iss >> vt;
					}

					if (iss.peek() == '/') {
						iss >> trash;
						//处理法线
						int vn;
						iss >> vn;
					}
				}
			}
			faces.push_back(face_indices);
		}
		else continue;
	}
}

int Model::nverts() const {
	return static_cast<int>(verts.size());
}

int Model::nfaces() const {
	return static_cast<int>(faces.size());
}

Vec3f Model::vert(const int i) const {
	return verts[i];
}

Vec3f Model::vert(const int iface, const int nthvert) const {
	return verts[faces[iface][nthvert]];
}



```

#### main.cpp

接下来要用读取到的数据画线了，但是我们仍然会遇到两个问题。

1、我们读取的坐标信息包含了 $$z$$ 轴，但是目前我们只知道设置 $$xy$$ 轴。

在第一章，我们进行的操作实际上是**沿 $$z$$ 轴方向的正交投影**，相当于直接把模型压扁到 $$xOy$$ 面上了。这操作很简单，也就是说我们不用管 $$z$$ 轴了，而且看起来效果还行。

* **公式**：
  $$
  x_{screen} = x_{model} \\
  y_{screen} = y_{model}
  $$

* **物理意义**：把三维物体垂直拍扁在屏幕上。

* **视觉效果**：没有透视（近大远小），只有轮廓。

代价就是信息的丢失，在只画线的时候感觉不明显，但是如果进入填充模式要给三角形上色，就会出现问题。这部分我们会在后面的章节解决。

2、我们使用TGAImage的方法`void set(const int x, const int y, const TGAColor &c)`来设置 $$(x, y)$$ 点的像素颜色，传入的xy需要是整形，但是我们读取到的顶点坐标都是浮点型。而且打开obj文件会发现顶点的坐标都是在 $$[-1, 1]$$ 范围内，如果强行转换成整型，就会都变成 0 。

为了让它们正确显示，我们要把这些坐标对应到屏幕上。

```c++
//假如我初始规定了这样大小的屏幕
constexpr int width  = 800;
constexpr int height = 800;
TGAImage framebuffer(width, height, TGAImage::RGB);
```

要把 $$[-1, 1]$$ 范围内的 $$xy$$ 分别映射到 $$[0, width]$$ 和 $$[0, height]$$，我们需要进行以下操作：

```c++
int screen_x0 = static_cast<int>((v0.x + 1.0f) * width / 2.0f);
int screen_y0 = static_cast<int>((v0.y + 1.0f) * height / 2.0f);
int screen_x1 = static_cast<int>((v1.x + 1.0f) * width / 2.0f);
int screen_y1 = static_cast<int>((v1.y + 1.0f) * height / 2.0f);
```

`.obj` 文件里存储的顶点坐标 $(x, y, z)$，其实是模型空间 (Model Space) 的坐标，也被称为局部空间 (Local Space)。

为了让你彻底理解这意味着什么，以及它在后续课程中的位置，我们可以打个比方。

在 TinyRenderer 的第一章，其实做了一个巨大的简化。

完整的图形学流水线（Pipeline）是这样的：

1.  **模型空间 (Model Space)**：OBJ 文件里的原始数据。
    *   *(现状)*
2.  **世界空间 (World Space)**：你把人偶摆在房间的角落里。
    *   *需要乘以 Model Matrix (模型矩阵)*。
3.  **观察空间 (View Space / Camera Space)**：以摄像机为中心看这个世界。
    *   *需要乘以 View Matrix (视图矩阵)*。
4.  **裁剪空间 (Clip Space) / 归一化设备坐标 (NDC)**：把视野内的东西压缩到 $[-1, 1]$ 的立方体里。
    *   *需要乘以 Projection Matrix (投影矩阵)*。
5.  **屏幕空间 (Screen Space)**：把 $[-1, 1]$ 映射到像素坐标 $[0, 800]$。
    *   *(刚才写的 `(x+1)*width/2` 就是这一步)*

**现在的做法实际上是：**
假设模型就放在世界中心 $(0,0,0)$，摄像机正对着它，没有任何旋转或位移。
所以**跳过了中间所有的矩阵变换**，直接把**模型空间**强行当成了**NDC**（$[-1, 1]$ 范围），然后一步转换到了**屏幕空间**。

```c++
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

	//这里可以改成从命令行参数传入模型路径
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

```

### 结果与复盘

我们会得到以下图片：![diablo1](images/chap1/diablo1.png)

复盘待补