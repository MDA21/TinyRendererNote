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
			//blender的导出格式是 f  v/vt/vn
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


