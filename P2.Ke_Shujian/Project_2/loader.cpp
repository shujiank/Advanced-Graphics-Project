#include "loader.h"
#include <iostream>
#include <fstream>

using namespace std;

inline int StrToInt(const string &str) {
	int i;
	if (sscanf(str.c_str(), "%i", &i) == 1)
		return i;
	else return 0;
}

inline vector<string> split_string(const string& str, const string& split_str) {
	vector<string> stlv_string;
	string part_string("");
	string::size_type i;
	i = 0;
	while (i < str.size()) {
		if (split_str.find(str[i]) != string::npos) {
			stlv_string.push_back(part_string);
			part_string = "";
			while (split_str.find(str[i]) != string::npos) {
				++i;
			}
		}
		else {
			part_string += str[i];
			++i;
		}
	}
	if (!part_string.empty())
		stlv_string.push_back(part_string);
	return stlv_string;
}

Mesh* ObjLoader::load(const char* file) {
	ifstream infile(file);
	if (infile.fail()) {
		cout << "Error opening file " << file;
		return false;
	}
	Mesh* mesh = new Mesh;
	char current_line[1024];
	while (!infile.eof()) {
		infile.getline(current_line, 1024);
		switch (current_line[0]) {
		case 'v': {
			float x, y, z;
			switch (current_line[1]) {
			case 'n':
				sscanf(current_line, "vn %f %f %f", &x, &y, &z);
				mesh->dot_normalPerFace.push_back(Vec3f(x, y, z));
				break;
			case 't':
				//sscanf(current_line, "vt %f %f %f", &x,&y,&z); // z component is not used
				sscanf(current_line, "vt %f %f", &x, &y); // z component is not used
				mesh->dot_texture.push_back(Vec2f(x, y));
				break;
			default:
				sscanf(current_line, "v %f %f %f", &x, &y, &z);
				mesh->dot_vertex.push_back(Vec3f(x, y, z));
				break;
			}
		}
				  break;
		case 'f': {
			vector<string> faces = split_string(current_line, " \t.\r\n");
			int vnt[3][3] = { { -1, -1, -1 },{ -1, -1, -1 },{ -1, -1, -1 } };
			string::size_type begin, end;
			for (int i = 0; i < 2; i++) {
				//cout<<"f: "<<i<<" "<<faces[i+1]<<endl;
				begin = 0;
				int j = 0;
				do {
					end = faces[i + 1].find_first_of("/", begin);
					if (begin < end) {
						vnt[i][j] = StrToInt(faces[i + 1].substr(begin, end - begin)) - 1;
					}
					else
						vnt[i][j] = -1;
					begin = end + 1;
					j++;
				} while (end != string::npos);
			}

			for (unsigned int i = 3; i < faces.size(); i++) {
				//cout<<"f: "<<i-1<<" "<<faces[i]<<endl;
				begin = 0;
				int j = 0;
				do {
					end = faces[i].find_first_of("/", begin);
					if (begin < end) {
						vnt[2][j] = StrToInt(faces[i].substr(begin, end - begin)) - 1;
					}
					else
						vnt[2][j] = -1;
					begin = end + 1;
					j++;
				} while (end != string::npos);
				for (int j = 0; j < 3; j++) {
					//cout<<"\t"<<vnt[j][0]<<" "<<vnt[j][1]<<" "<<vnt[j][2]<<"\n";
					mesh->face_index_vertex.push_back(vnt[j][0]);
					if (vnt[j][1] != -1) mesh->face_index_texture.push_back(vnt[j][1]);
					if (vnt[j][2] != -1) mesh->face_index_normalPerFace.push_back(vnt[j][2]);
				}
				memcpy(&vnt[1], &vnt[2], sizeof(int) * 3);
			}
		}
				  break;

		default: break;
		}
	}
	return mesh;
}