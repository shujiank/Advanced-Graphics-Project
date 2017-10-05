#include <stdio.h>
#include <fstream>
#include <ctime>
#include "loader.h"
#include "BoundingBox.h"
#include "NormalCalculation.h"
#include "noise.h"
#include <windows.h>
#include <wingdi.h>

#define PI 3.14159265

// timer
time_t start_time, current_time;

// check points aabboxes
AABBox start_position_aabbox, finish_position_aabbox;
bool isStart = false, isFinish = false;

// normal mode
int normal_mode = 0; // 0 for per vertex normal, 1 for per face normal, 2 for per vertex normal weighted

// plane mode
int plane_mode = 2; // 0 for flat, 1 for multiscale, 2 for marble

// texture mode
bool texture_mode = true;

// noise
ImprovedNoise pNoise = ImprovedNoise();

// navigation mode
int navigation_mode = 0; // 0 for light source, 1 for camera
int degree_around_terrain = 0;
float terrain_move_y = 0;
float radius_around_terrain = 30;

// view mode
int view_mode = 1; // 0 for overview, 1 for first person

// material mode
int material_mode = 0; // 0 for design, 1 for diffuse, 2 for specular

// visualization
bool visualization = false; 

// menu parameters
bool collision = false;
int bbox = 0; // 0 for bbox off, 1 for aabbox, 2 for orited bbox
bool display_list = true;
int shading = 0; // 0 for flat, 1 for smooth

				 // display lists
GLuint wall_DL, lego_DL, car_DL, plane_DL, sky_DL, cylinder_DL;

Mesh* plane = 0;
Mesh* sky = 0;
Mesh* cylinder = 0;

// lighting parameters
GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_position[] = { 0.0, 50.0, -60.0, 1.0 };
int light_degree = 200;

// window parameters
GLint winId;
int position_x = 0;
int position_y = 0;
int width = 1200;
int height = 600;


// controling parameters
int mouse_button;
int mouse_x = 0;
int mouse_y = 0;
float scale = 0.3;
float x_angle = 0.0;
float y_angle = 0.0;

// car parameters
Mesh* car_model = 0;
AABBox car_aabbox, car_orig_aabbox;
float car_x = 0.0; // start place
float car_y = -1.5;
float car_z = -2.0;
float rotate_x = 0;
float rotate_y = 0;
float car_angle = 180;
float car_scale = 0.02;

// camera parameters

float camera_x = 10.0;
float camera_y = 50.0;
float camera_z = -16.0;
float center_x = 10.0;
float center_y = 5;
float center_z = -16;
float up_x = 0.0;
float up_y = 0.0;
float up_z = -1.0;

// perspective projection parameters
GLfloat fieldofview = 90.0;
GLfloat aspect = 1.0;
GLfloat nearPlane = 1;
GLfloat farPlane = 10000.0;

// textures with BITMAPS
GLuint textureArray[8];


// lego models
struct lego_parameters {
	Vec3f lego_coordinates;
	Vec3f lego_rotate;
	float lego_angle;
};

Mesh* lego_model = 0;
vector<AABBox> lego_aabboxes;
AABBox lego_orig_aabbox;
vector<lego_parameters> lego_param;
float lego_scale = 0.3;

// lybrinth
Mesh* wall;
vector<AABBox> aabboxes;

// models
static const char* modelfile[] = {
	"../data/f-16.obj",
	"../data/al.obj",
	"../data/dolphins.obj",
	"../data/porsche.obj",
	"../data/rose+vase.obj",
	"../data/flowers.obj",
	"../data/LEGO_Man.obj"
};

void move_all_lego_models();
void createDisplayList();

// noise function
float t_scale(float x) {
	return (x + 1) / 2;
}

Vec3f marbleMap(float a) {
	Vec3f black = Vec3f(0, 0, 0);
	Vec3f color = Vec3f(1, 1, 1);
	return ((1 - a) * color + a * black);
}

Vec3f blueMap(float a) {
	Vec3f black = Vec3f(1, 1, 1);
	Vec3f color = Vec3f(0.8, 0.34, 0.3);
	return ((1 - a) * color + a * black);
}

Vec3f valleyMap(float a) {
	Vec3f brown = Vec3f(0.09, 0.20, 0.36);
	Vec3f green = Vec3f(0, 0.31, 0);
	//Vec3f brown = Vec3f(0.3, 0.3, 0.24);
	//Vec3f green = Vec3f(0.19, 0.24, 0.094);
	return ((1 - a) * brown + a * green);
}

float noiseParameter(float u, float v) {
	float temp = 0;
	for (int i = 1; i <= 7; i++) {
		temp += pow(1.5, (double)-i) * pNoise.noise(pow(2, (double)i) * u, pow(2, (double)i) * v, 11.5);
	}
	return temp;
}


// Load a DIB/BMP file from disk.
GLubyte *LoadDIBitmap(const char *filename, BITMAPINFO **info) {
	FILE *fp;      // open file pointer
	GLubyte * bits; // bitmap pixel bits
	int bitsize;   // Size of bitmap
	int infosize;  // Size of header information
	BITMAPFILEHEADER header; // File header
							 // try opening the file; use "rb" mode to read this *binary* file.
	if ((fp = fopen(filename, "rb")) == NULL)
		return (NULL);
	// read the file header and any following bitmap information.
	if (fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1) {
		// Couldn't read the file header - return NULL.
		fclose(fp);
		return (NULL);
	}
	// Check for BM reversed.
	if (header.bfType != 'MB') {
		// not a bitmap file - return NULL.
		fclose(fp);
		return (NULL);
	}
	infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
	if ((*info = (BITMAPINFO *)malloc(infosize)) == NULL) {
		// couldn't allocate memory for bitmap info - return NULL.
		fclose(fp);
		return (NULL);
	}
	if (fread(*info, 1, infosize, fp) < infosize) {
		// Couldn't read the bitmap header - return NULL.
		free(*info);
		fclose(fp);
		return (NULL);
	}
	// Now that we have all the header info read in, allocate memory for the bitmap and read *it* in.
	if ((bitsize = (*info)->bmiHeader.biSizeImage) == 0)
		bitsize = ((*info)->bmiHeader.biWidth*(*info)->bmiHeader.biBitCount + 7) / 8 * abs((*info)->bmiHeader.biHeight);
	if ((bits = (GLubyte *)malloc(bitsize)) == NULL) {
		// Couldn't allocate memory - return NULL!
		free(*info);
		fclose(fp);
		return (NULL);
	}
	if (fread(bits, 1, bitsize, fp) < bitsize) {
		// couldn't read bitmap - free memory and return NULL!
		free(*info);
		free(bits);
		fclose(fp);
		return (NULL);
	}
	// OK, everything went fine - return the allocated bitmap.
	fclose(fp);
	return (bits);
}

void codedTexture(UINT textureArray[], int n, int m) {
	const int TexHeight = 128;
	const int TexWidth = 128;
	// create texture in memory
	GLubyte textureImage[TexHeight][TexWidth][4];
	for (int i = 0; i < TexHeight; i++)
		for (int j = 0; j < TexWidth; j++) {
			Vec3f pixelColor;
			if (m == 0) {
				pixelColor = blueMap(t_scale(noiseParameter((float)i / TexHeight, (float)j / TexWidth)));
			}
			else if (m == 1) {
				pixelColor = marbleMap(t_scale(pNoise.perlinMarble((float)i / TexHeight, (float)j / TexWidth)));
				//cout << pixelColor[0] << ", " << pixelColor[1] << ", " << pixelColor[2] << endl;
			}

			textureImage[i][j][0] = pixelColor[0] * 255; // red(blule) value from 0 to 255 
			textureImage[i][j][1] = pixelColor[1] * 255; // green value from 0 to 255 
			textureImage[i][j][2] = pixelColor[2] * 255; // blue(red) value from 0 to 255 

														 //textureImage[i][j][0] = 255; // red(blule) value from 0 to 255 
														 //textureImage[i][j][1] = 0; // green value from 0 to 255 
														 //textureImage[i][j][2] = 0; // blue(red) value from 0 to 255 
			textureImage[i][j][3] = 100; // alpha value from 0 to 255 
										 //cout << textureImage[i][j][0] << ", " << textureImage[i][j][1] << ", " << textureImage[i][j][2] << endl;

		}
	// setup texture
	glGenTextures(1, &textureArray[n]);
	glBindTexture(GL_TEXTURE_2D, textureArray[n]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // must set to 1 for compact data
										   // glTexImage2D Whith size and minification
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TexWidth, TexHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, textureImage);

}

// bmpTexture
void bmpTexture(UINT textureArray[], LPSTR file, int n) {
	BITMAPINFO *bitmapInfo; // Bitmap information
	GLubyte    *bitmapBits; // Bitmap data
	if (!file) {
		std::cout << "texture file not found!" << std::endl;
		return;
	}
	bitmapBits = LoadDIBitmap(file, &bitmapInfo);
	glGenTextures(1, &textureArray[n]);
	glBindTexture(GL_TEXTURE_2D, textureArray[n]);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // must set to 1 for compact data
										   // glTexImage2D Whith size and minification
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bitmapInfo->bmiHeader.biWidth, bitmapInfo->bmiHeader.biHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bitmapBits);
}


// wall
Mesh* create_wall(float x, float y, float z) {
	Mesh* m = new Mesh;
	m->dot_vertex.push_back(Vec3f(x + -0.05, y + -2, z + 2));
	m->dot_vertex.push_back(Vec3f(x + 0.05, y + -2, z + 2));
	m->dot_vertex.push_back(Vec3f(x + 0.05, y + 2, z + 2));
	m->dot_vertex.push_back(Vec3f(x + -0.05, y + 2, z + 2));
	m->dot_vertex.push_back(Vec3f(x + -0.05, y + -2, z + -2));
	m->dot_vertex.push_back(Vec3f(x + 0.05, y + -2, z + -2));
	m->dot_vertex.push_back(Vec3f(x + 0.05, y + 2, z + -2));
	m->dot_vertex.push_back(Vec3f(x + -0.05, y + 2, z + -2));

	// face 0
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(1);
	m->face_index_vertex.push_back(2);

	// face 1
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(2);
	m->face_index_vertex.push_back(3);

	// face 2
	m->face_index_vertex.push_back(1);
	m->face_index_vertex.push_back(5);
	m->face_index_vertex.push_back(6);

	// face 3
	m->face_index_vertex.push_back(1);
	m->face_index_vertex.push_back(6);
	m->face_index_vertex.push_back(2);

	// face 4
	m->face_index_vertex.push_back(4);
	m->face_index_vertex.push_back(6);
	m->face_index_vertex.push_back(5);

	// face 5
	m->face_index_vertex.push_back(4);
	m->face_index_vertex.push_back(7);
	m->face_index_vertex.push_back(6);

	// face 6
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(3);
	m->face_index_vertex.push_back(7);

	// face 7
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(7);
	m->face_index_vertex.push_back(4);

	// face 8
	m->face_index_vertex.push_back(2);
	m->face_index_vertex.push_back(6);
	m->face_index_vertex.push_back(7);

	// face 9
	m->face_index_vertex.push_back(2);
	m->face_index_vertex.push_back(7);
	m->face_index_vertex.push_back(3);

	// face 10
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(4);
	m->face_index_vertex.push_back(5);

	// face 11
	m->face_index_vertex.push_back(0);
	m->face_index_vertex.push_back(5);
	m->face_index_vertex.push_back(1);

	m->dot_texture.push_back(Vec2<GLfloat>(0.0, 1.0));
	m->dot_texture.push_back(Vec2<GLfloat>(1.0, 1.0));
	m->dot_texture.push_back(Vec2<GLfloat>(1.0, 0.0));
	m->dot_texture.push_back(Vec2<GLfloat>(0.0, 0.0));
	for (int t = 0; t<6; t++) {
		m->face_index_texture.push_back(0);//0
		m->face_index_texture.push_back(2);//1
		m->face_index_texture.push_back(1);//2
		m->face_index_texture.push_back(0);//0
		m->face_index_texture.push_back(3);//2
		m->face_index_texture.push_back(2);//3
	}

	return m;
}

// creating a triangulated plane
Mesh* create_plane(int arena_width, int arena_depth, int arena_cell) {
	Mesh *me = new Mesh;
	int n = arena_width / arena_cell;
	int m = arena_depth / arena_cell;

	float perlinY;
	// vertices
	for (int i = 0; i<n; i++) {
		for (int j = 0; j < m; j++) {
			switch (plane_mode)
			{
			case 0:
				perlinY = 0;
				break;
			case 1:
				perlinY = pNoise.perlinMultiscale(i, j);
				break;
			case 2:
				perlinY = pNoise.perlinMarble(i, j, 0.001, 100, 4);
				//cout << "marble" << endl;
				break;
			default:
				break;
			}
			
			me->dot_vertex.push_back(Vec3<GLfloat>(i*arena_cell, perlinY, j*arena_cell));
			me->dot_texture.push_back(Vec2<GLfloat>((float)i*arena_cell / arena_width, (float)j*arena_cell / arena_depth));
		}
	}

	// faces
	for (int i = 0; i<(n*m) - m; i++) {
		if ((i + 1) % n == 0) continue;
		me->face_index_vertex.push_back(i); me->face_index_vertex.push_back(i + 1);
		me->face_index_vertex.push_back(i + n);
		me->face_index_vertex.push_back(i + 1); me->face_index_vertex.push_back(i + n + 1);
		me->face_index_vertex.push_back(i + n);

		me->face_index_texture.push_back(i); me->face_index_texture.push_back(i + 1);
		me->face_index_texture.push_back(i + n);
		me->face_index_texture.push_back(i + 1); me->face_index_texture.push_back(i + n + 1);
		me->face_index_texture.push_back(i + n);
	}
	return me;
}

Mesh* createSky(int radius, int high) {
	Mesh *mesh = new Mesh;
	int r, h;
	for (int i = 0; i <= 90; i += 10) {
		r = radius * cos(i * PI / 180);
		h = high * sin(i * PI / 180);
		for (int degree = 0; degree <= 360; degree += 10) {
			mesh->dot_vertex.push_back(Vec3<GLfloat>(r * sin(degree * PI / 180), h, r * cos(degree * PI / 180)));
			mesh->dot_texture.push_back(Vec2<GLfloat>((float)degree / 360, (float)h / high));
		}
	}

	int len = mesh->dot_vertex.size();
	
	for (int i = 0; i < len - 37; i++) {
		if (i % 37 == 36) {
			mesh->face_index_vertex.push_back(i);
			mesh->face_index_vertex.push_back(i + 37);
			mesh->face_index_vertex.push_back(i + 1 - 37);
			mesh->face_index_vertex.push_back(i + 37);
			mesh->face_index_vertex.push_back(i + 37 + 1 - 37);
			mesh->face_index_vertex.push_back(i + 1 - 37);

			mesh->face_index_texture.push_back(i);
			mesh->face_index_texture.push_back(i + 37);
			mesh->face_index_texture.push_back(i + 1 - 37);
			mesh->face_index_texture.push_back(i + 37);
			mesh->face_index_texture.push_back(i + 37 + 1 - 37);
			mesh->face_index_texture.push_back(i + 1 - 37);
		}
		else {
			mesh->face_index_vertex.push_back(i);
			mesh->face_index_vertex.push_back(i + 37);
			mesh->face_index_vertex.push_back(i + 1);
			mesh->face_index_vertex.push_back(i + 37);
			mesh->face_index_vertex.push_back(i + 37 + 1);
			mesh->face_index_vertex.push_back(i + 1);

			mesh->face_index_texture.push_back(i);
			mesh->face_index_texture.push_back(i + 37);
			mesh->face_index_texture.push_back(i + 1);
			mesh->face_index_texture.push_back(i + 37);
			mesh->face_index_texture.push_back(i + 37 + 1);
			mesh->face_index_texture.push_back(i + 1);
		}
	}
	

	return mesh;
}

Mesh* createCylinder(int radius, int high) {
	Mesh *mesh = new Mesh;
	// bottom vertices
	for (int degree = 0; degree <= 360; degree += 10) {
		mesh->dot_vertex.push_back(Vec3<GLfloat>(radius * sin(degree * PI / 180), 0, radius * cos(degree * PI / 180)));
		mesh->dot_texture.push_back(Vec2<GLfloat>(((float)degree / 360), 0.0));
	}
	// top vertices
	for (int degree = 0; degree <= 360; degree += 10) {
		mesh->dot_vertex.push_back(Vec3<GLfloat>(radius * sin(degree * PI / 180), high, radius * cos(degree * PI / 180)));
		mesh->dot_texture.push_back(Vec2<GLfloat>(((float)degree / 360), 1));
	}

	// bottom center
	mesh->dot_vertex.push_back(Vec3<GLfloat>(0, 0, 0));
	// top center
	mesh->dot_vertex.push_back(Vec3<GLfloat>(0, high, 0));


	int len = mesh->dot_vertex.size();
	int t_len = mesh->dot_texture.size();

	// bottom cycle
	for (int i = 0; i < 37; i++) {
		mesh->face_index_vertex.push_back(i);
		mesh->face_index_vertex.push_back((i + 1) % 37);
		mesh->face_index_vertex.push_back(len - 2);
		mesh->face_index_texture.push_back(3);
		mesh->face_index_texture.push_back(0);
		mesh->face_index_texture.push_back(2);
	}

	// top cycle
	for (int i = 0; i < 37; i++) {
		mesh->face_index_vertex.push_back(i + 37);
		mesh->face_index_vertex.push_back((i + 1) % 37 + 37);
		mesh->face_index_vertex.push_back(len - 1);
		mesh->face_index_texture.push_back(3);
		mesh->face_index_texture.push_back(0);
		mesh->face_index_texture.push_back(2);
	}

	// body
	for (int i = 0; i < 37; i++) {
		mesh->face_index_vertex.push_back(i);
		mesh->face_index_vertex.push_back((i + 1) % 37);
		mesh->face_index_vertex.push_back(i + 37);
		mesh->face_index_vertex.push_back((i + 1) % 37);
		mesh->face_index_vertex.push_back((i + 1) % 37 + 37);
		mesh->face_index_vertex.push_back(i + 37);

		mesh->face_index_texture.push_back(i);
		mesh->face_index_texture.push_back((i + 1) % 37);
		mesh->face_index_texture.push_back(i + 37);
		mesh->face_index_texture.push_back((i + 1) % 37);
		mesh->face_index_texture.push_back((i + 1) % 37 + 37);
		mesh->face_index_texture.push_back(i + 37);
	}

	return mesh;
}


// draw
GLuint meshToDisplayList(Mesh* m, int id) {
	GLuint listID = glGenLists(id + 1);
	glNewList(listID, GL_COMPILE);
	if (id > 0 && texture_mode) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, textureArray[id - 1]);
	}
	
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
		// PER VERTEX NORMALS
		if (normal_mode == 0) {
			if ((!m->dot_normalPerVertex.empty() && !m->face_index_normalPerVertex.empty())) {
				glNormal3fv(&m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].x);
			}
		}
		else if (normal_mode == 1) {
			if ((!m->dot_normalPerFace.empty() && !m->face_index_normalPerFace.empty())) {
				glNormal3fv(&m->dot_normalPerFace[m->face_index_normalPerFace[i]].x);
			}
		}
		else if (normal_mode == 2) {
			if ((!m->dot_normalPerVertexWeighted.empty() && !m->face_index_normalPerVertexWeighted.empty())) {
				glNormal3fv(&m->dot_normalPerVertexWeighted[m->face_index_normalPerVertexWeighted[i]].x);
			}
		}
		
		// TEXTURES
		if (!m->dot_texture.empty() && !m->face_index_texture.empty()) {
			glTexCoord2fv(&m->dot_texture[m->face_index_texture[i]].x);
		}
		// COLOR
		Vec3f offset = (m->dot_vertex[m->face_index_vertex[i]]);
		// VERTEX
		glColor3f(fabs(sin(offset.x)), fabs(cos(offset.y)), fabs(offset.z));
		glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
	}

	glEnd();

	if (id > 0 && texture_mode) {
		glDisable(GL_TEXTURE_2D);
	}
	
	if (visualization) {
		glBegin(GL_LINES);
		glColor3f(1, 1, 1);
		if (normal_mode == 0) {
			for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
				glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
				glVertex3f(m->dot_vertex[m->face_index_vertex[i]].x + 2 * m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].x,
					m->dot_vertex[m->face_index_vertex[i]].y + 2 * m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].y,
					m->dot_vertex[m->face_index_vertex[i]].z + 2 * m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].z);
			}
		}
		else if (normal_mode == 1) {
			Vec3f center = Vec3f(0, 0, 0);
			for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
				center += m->dot_vertex[m->face_index_vertex[i]];
				if ((i + 1) % 3 == 0) {
					center /= 3;
					glVertex3fv(&center.x);
					glVertex3f(center.x + 2 * m->dot_normalPerFace[m->face_index_normalPerFace[i]].x,
						center.y + 2 * m->dot_normalPerFace[m->face_index_normalPerFace[i]].y,
						center.z + 2 * m->dot_normalPerFace[m->face_index_normalPerFace[i]].z);
					center = Vec3f(0, 0, 0);
				}
			}
		}
		else if (normal_mode == 2) {
			for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
				glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
				glVertex3f(m->dot_vertex[m->face_index_vertex[i]].x + 2 * m->dot_normalPerVertexWeighted[m->face_index_normalPerVertexWeighted[i]].x,
					m->dot_vertex[m->face_index_vertex[i]].y + 2 * m->dot_normalPerVertexWeighted[m->face_index_normalPerVertexWeighted[i]].y,
					m->dot_vertex[m->face_index_vertex[i]].z + 2 * m->dot_normalPerVertexWeighted[m->face_index_normalPerVertexWeighted[i]].z);
			}
		}
	}
	
	
	glEnd();
	
	
	glEndList();
	return listID;
}


void draw_wall(Mesh* m) {
	// if choose display list mode
	if (display_list) {
		//cout << "call display list" << endl;
		glCallList(wall_DL);
		return;
	}

	int id = 1;
	if (texture_mode) {
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glBindTexture(GL_TEXTURE_2D, textureArray[id - 1]);
	}
	
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
		// PER VERTEX NORMALS
		if ((!m->dot_normalPerVertex.empty() && !m->face_index_normalPerVertex.empty())) {
			glNormal3fv(&m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].x);
		}
		// TEXTURES
		if (!m->dot_texture.empty() && !m->face_index_texture.empty()) {
			glTexCoord2fv(&m->dot_texture[m->face_index_texture[i]].x);
		}
		// COLOR
		Vec3f offset = (m->dot_vertex[m->face_index_vertex[i]]);
		// VERTEX
		glColor3f(fabs(sin(offset.x)), fabs(cos(offset.y)), fabs(offset.z));
		glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
	}

	glEnd();
	if (texture_mode) {
		glDisable(GL_TEXTURE_2D);
	}
	
}

void draw_all_walls() {
	AABBox originAABBox;
	AABBox aabbox;
	glm::mat4 myMatrix;
	//originAABBox.min_v = mat * originAABBox.min_v;
	originAABBox.mesh = wall;
	calc_aabbox(wall, originAABBox.min_v, originAABBox.max_v);

	// rows
	glPushMatrix();
	glTranslatef(-2, 0, -16);
	glScalef(1, 1, 7);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0, -16)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 7));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(2, 0, -8);
	glScalef(1, 1, 3);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, -8)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 3));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2, 0, -22);
	glScalef(1, 1, 2);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, -22)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 2));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(6, 0, -18);
	glScalef(1, 1, 6);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(6, 0, -18)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 6));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(18, 0, -16);
	glScalef(1, 1, 5);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(18, 0, -16)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 5));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(22, 0, -16);
	glScalef(1, 1, 7);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(22, 0, -16)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 7));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	// columns
	glPushMatrix();
	glTranslatef(12, 0, -2);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 5);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(12, 0, -2)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 5));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14, 0, -6);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 2);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(14, 0, -6)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 2));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(10, 0, -10);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 2);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(10, 0, -10)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 2));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -18);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 1);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -18)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 1));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14, 0, -26);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 2);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(14, 0, -26)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 2));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(10, 0, -30);
	glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 6);

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(10, 0, -30)) *
		glm::rotate(glm::mat4(1.0f), (float)(90.0f * PI / 180), glm::vec3(0, -1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(1, 1, 6));

	// transform axis-aligned bounding box
	aabbox = originAABBox;
	transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
	aabboxes.push_back(aabbox);

	draw_wall(wall);
	glPopMatrix();
}

void draw_plane() {
	glPushMatrix();
	glTranslatef(-50, -3, -60);
	glScalef(0.2, 0.2, 0.2);
	glCallList(plane_DL);
	glPopMatrix();
}

bool is_start() {
	if ((car_aabbox.min_v.x <= start_position_aabbox.max_v.x && car_aabbox.max_v.x >= start_position_aabbox.min_v.x)
		&& (car_aabbox.min_v.y <= start_position_aabbox.max_v.y && car_aabbox.max_v.y >= start_position_aabbox.min_v.y)
		&& (car_aabbox.min_v.z <= start_position_aabbox.max_v.z && car_aabbox.max_v.z >= start_position_aabbox.min_v.z)) {
		return true;
	}
	return false;
}

bool is_finish() {
	if ((car_aabbox.min_v.x <= finish_position_aabbox.max_v.x && car_aabbox.max_v.x >= finish_position_aabbox.min_v.x)
		&& (car_aabbox.min_v.y <= finish_position_aabbox.max_v.y && car_aabbox.max_v.y >= finish_position_aabbox.min_v.y)
		&& (car_aabbox.min_v.z <= finish_position_aabbox.max_v.z && car_aabbox.max_v.z >= finish_position_aabbox.min_v.z)) {
		return true;
	}
	return false;
}

void draw_checkpoints() {
	// start position
	GLfloat mat_emission1[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission1);
	GLfloat mat_ambient1[] = { 0, 0, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient1);
	GLfloat mat_diffuse1[] = { 0, 0, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse1);
	GLfloat mat_specular1[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular1);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	glColor3f(0, 0, 1);
	glBegin(GL_TRIANGLES);
	glVertex3f(-2, -1.9, 0);
	glVertex3f(2, -1.9, 0);
	glVertex3f(2, -1.9, -4);
	glVertex3f(2, -1.9, -4);
	glVertex3f(-2, -1.9, -4);
	glVertex3f(-2, -1.9, 0);
	glEnd();
	start_position_aabbox.min_v = Vec3f(-2, -2, -4);
	start_position_aabbox.max_v = Vec3f(2, 2, 0);

	// finish point

	GLfloat mat_emission2[] = { 1, 0.87, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission2);
	GLfloat mat_ambient2[] = { 1, 0.87, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
	GLfloat mat_diffuse2[] = { 1, 0.87, 0, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
	GLfloat mat_specular2[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	glColor3f(1, 0, 0);
	glBegin(GL_TRIANGLES);
	glVertex3f(18, -1.9, -2);
	glVertex3f(22, -1.9, -2);
	glVertex3f(22, -1.9, -6);
	glVertex3f(22, -1.9, -6);
	glVertex3f(18, -1.9, -6);
	glVertex3f(18, -1.9, -2);
	glEnd();
	finish_position_aabbox.min_v = Vec3f(18, -2, -6);
	finish_position_aabbox.max_v = Vec3f(22, 2, -2);

	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
	GLfloat mat_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
	GLfloat mat_diffuse0[] = { 0.8, 0.8, 0.8, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
	GLfloat mat_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);
}

// different collision detection routines
bool lego_collision_with_each_other(unsigned self) {
	for (unsigned i = 0; i < lego_aabboxes.size(); i++) {
		if (i != self) {
			if ((lego_aabboxes[self].min_v.x <= lego_aabboxes[i].max_v.x && lego_aabboxes[self].max_v.x >= lego_aabboxes[i].min_v.x)
				&& (lego_aabboxes[self].min_v.y <= lego_aabboxes[i].max_v.y && lego_aabboxes[self].max_v.y >= lego_aabboxes[i].min_v.y)
				&& (lego_aabboxes[self].min_v.z <= lego_aabboxes[i].max_v.z && lego_aabboxes[self].max_v.z >= lego_aabboxes[i].min_v.z)) {
				return true;
			}
		}
	}
	return false;
}

bool lego_collision_detection_with_car(unsigned i) {
	return ((car_aabbox.min_v.x <= lego_aabboxes[i].max_v.x && car_aabbox.max_v.x >= lego_aabboxes[i].min_v.x)
		&& (car_aabbox.min_v.y <= lego_aabboxes[i].max_v.y && car_aabbox.max_v.y >= lego_aabboxes[i].min_v.y)
		&& (car_aabbox.min_v.z <= lego_aabboxes[i].max_v.z && car_aabbox.max_v.z >= lego_aabboxes[i].min_v.z));
}

bool car_collision_detection() {
	// for walls 
	for (unsigned i = 0; i < aabboxes.size(); i++) {
		if ((car_aabbox.min_v.x <= aabboxes[i].max_v.x && car_aabbox.max_v.x >= aabboxes[i].min_v.x)
			&& (car_aabbox.min_v.y <= aabboxes[i].max_v.y && car_aabbox.max_v.y >= aabboxes[i].min_v.y)
			&& (car_aabbox.min_v.z <= aabboxes[i].max_v.z && car_aabbox.max_v.z >= aabboxes[i].min_v.z)) {
			return true;
		}
	}

	// for lego models
	for (unsigned i = 0; i < lego_aabboxes.size(); i++) {
		if (lego_collision_detection_with_car(i)) {
			return true;
		}
	}
	return false;
}

// draw aabboxes
void draw_all_aabboxes() {
	for (unsigned i = 0; i < aabboxes.size(); i++) {
		draw_aabbox(aabboxes[i]);
	}
	draw_aabbox(car_aabbox);

	for (unsigned i = 0; i < lego_aabboxes.size(); i++) {
		draw_aabbox(lego_aabboxes[i]);
	}
}

void calculate_lego_models_aabbox() {
	glm::mat4 myMatrix;
	AABBox aabbox;
	for (unsigned i = 0; i < lego_param.size(); i++) {
		aabbox = lego_orig_aabbox;
		myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(lego_param[i].lego_coordinates.x, lego_param[i].lego_coordinates.y, lego_param[i].lego_coordinates.z)) *
			glm::rotate(glm::mat4(1.0f), (float)(lego_param[i].lego_angle * PI / 180), glm::vec3(0, 1, 0)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(lego_scale, lego_scale, lego_scale));

		transform_aabbox(myMatrix, aabbox.min_v, aabbox.max_v);
		lego_aabboxes.push_back(aabbox);
	}
}

// load models
Mesh* load_car(const char* name) {
	Mesh *m = ObjLoader::load(name);
	//cout << car_model->m_vi.size() / 3 << endl;
	car_aabbox.mesh = m;
	calc_aabbox(m, car_aabbox.min_v, car_aabbox.max_v);
	car_orig_aabbox = car_aabbox;

	glm::mat4 myMatrix;
	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car_x, car_y, car_z)) *
		glm::rotate(glm::mat4(1.0f), (float)(car_angle * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(car_scale, car_scale, car_scale));

	transform_aabbox(myMatrix, car_aabbox.min_v, car_aabbox.max_v);
	return m;
}

Mesh* load_lego(const char* name) {
	Vec3f bbxMin, bbxMax;
	Mesh* m = ObjLoader::load(name);
	//cout << lego_model->m_vi.size() / 3 << endl;
	calc_aabbox(m, bbxMin, bbxMax);
	lego_orig_aabbox.mesh = m;
	lego_orig_aabbox.min_v = bbxMin;
	lego_orig_aabbox.max_v = bbxMax;
	return m;
}

void draw_lego_gl(Mesh* m, unsigned i) {
	if (display_list) {
		//cout << "call display list" << endl;
		glCallList(lego_DL);
		return;
	}
	//glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//glBindTexture(GL_TEXTURE_2D, textureArray[id - 1]);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
		// PER VERTEX NORMALS
		if ((!m->dot_normalPerVertex.empty() && !m->face_index_normalPerVertex.empty())) {
			glNormal3fv(&m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].x);
		}
		// TEXTURES
		if (!m->dot_texture.empty() && !m->face_index_texture.empty()) {
			glTexCoord2fv(&m->dot_texture[m->face_index_texture[i]].x);
		}
		// COLOR
		Vec3f offset = (m->dot_vertex[m->face_index_vertex[i]]);
		// VERTEX
		glColor3f(fabs(sin(offset.x)), fabs(cos(offset.y)), fabs(offset.z));
		glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
	}

	glEnd();
	//glDisable(GL_TEXTURE_2D);
}

void draw_lego() {
	for (unsigned i = 0; i < lego_param.size(); i++) {
		glPushMatrix();
		glTranslatef(lego_param[i].lego_coordinates.x, lego_param[i].lego_coordinates.y, lego_param[i].lego_coordinates.z);
		glScalef(lego_scale, lego_scale, lego_scale);
		glRotatef(lego_param[i].lego_angle, 0.0f, 1.0f, 0.0f);
		draw_lego_gl(lego_model, i);
		glPopMatrix();
	}
}

void draw_car_gl(Mesh* m) {
	if (display_list) {
		//cout << "call display list" << endl;
		glCallList(car_DL);
		return;
	}
	//glEnable(GL_TEXTURE_2D);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//glBindTexture(GL_TEXTURE_2D, textureArray[id - 1]);
	glBegin(GL_TRIANGLES);
	for (unsigned int i = 0; i < m->face_index_vertex.size(); i++) {
		// PER VERTEX NORMALS
		if ((!m->dot_normalPerVertex.empty() && !m->face_index_normalPerVertex.empty())) {
			glNormal3fv(&m->dot_normalPerVertex[m->face_index_normalPerVertex[i]].x);
		}
		// TEXTURES
		if (!m->dot_texture.empty() && !m->face_index_texture.empty()) {
			glTexCoord2fv(&m->dot_texture[m->face_index_texture[i]].x);
		}
		// COLOR
		Vec3f offset = (m->dot_vertex[m->face_index_vertex[i]]);
		// VERTEX
		glColor3f(fabs(sin(offset.x)), fabs(cos(offset.y)), fabs(offset.z));
		glVertex3fv(&m->dot_vertex[m->face_index_vertex[i]].x);
	}

	glEnd();
	//glDisable(GL_TEXTURE_2D);
}

void draw_car() {

	glPushMatrix();

	glTranslatef(car_x, car_y, car_z);
	glScalef(car_scale, car_scale, car_scale);
	glRotatef(car_angle, 0.0f, 1.0f, 0.0f);

	draw_car_gl(car_model);

	glPopMatrix();
}

void initialize_lego_models() {
	// initialization of lego model positions
	lego_parameters lego_p;
	// lego model 0
	lego_p.lego_angle = 0;
	lego_p.lego_coordinates = Vec3f(11.5, -1.8, -20);
	lego_param.push_back(lego_p);

	// lego model 1
	lego_p.lego_angle = 0;
	lego_p.lego_coordinates = Vec3f(lego_param[0].lego_coordinates.x + 3.5 * sin(lego_p.lego_angle * PI / 180),
		-1.8,
		lego_param[0].lego_coordinates.z + 3.5 * cos(lego_p.lego_angle * PI / 180));
	lego_param.push_back(lego_p);

	// lego model 2
	lego_p.lego_angle = 0;
	lego_p.lego_coordinates = Vec3f(lego_param[1].lego_coordinates.x + 1.5 * sin(lego_p.lego_angle * PI / 180),
		-1.8,
		lego_param[1].lego_coordinates.z + 1.5 * cos(lego_p.lego_angle * PI / 180));
	lego_param.push_back(lego_p);

	// lego model 3
	lego_p.lego_angle = 0;
	lego_p.lego_coordinates = Vec3f(13, -1.8, -14);
	lego_param.push_back(lego_p);

	// lego model 4
	lego_p.lego_angle = 0;
	lego_p.lego_coordinates = Vec3f(lego_param[3].lego_coordinates.x + 1.5 * sin(lego_p.lego_angle * PI / 180),
		-1.8,
		lego_param[3].lego_coordinates.z + 1.5 * cos(lego_p.lego_angle * PI / 180));
	lego_param.push_back(lego_p);

}

void changePlane() {
	plane = create_plane(500, 500, 10);
	calculateNormalPerFace(plane);
	calculateNormalPerVertex(plane);
	calculateNormalPerVertexWeighted(plane);
	plane_DL = meshToDisplayList(plane, 2);
}

void menuEvent(int menuId) {
	switch (menuId) {
	case 0:
		camera_x = 10.0;
		camera_y = 50.0;
		camera_z = -16.0;
		center_x = 10.0;
		center_y = 5;
		center_z = -16;
		up_x = 0.0;
		up_y = 0.0;
		up_z = -1.0;
		break;
	case 1:
		center_x = 10.0;
		center_y = 5;
		center_z = -5;
		degree_around_terrain = 0;
		terrain_move_y = 0;
		radius_around_terrain = 30;
		camera_x = center_x + radius_around_terrain * sin(degree_around_terrain * PI / 180);
		camera_y = center_y + 20;
		camera_z = center_z + radius_around_terrain * cos(degree_around_terrain * PI / 180);
		up_x = 0.0;
		up_y = 1.0;
		up_z = 0.0;
		view_mode = 0;
		break;
	case 2:
		display_list = true;
		break;
	case 3:
		display_list = false;
		break;
	case 4:
		glShadeModel(GL_FLAT);
		break;
	case 5:
		glShadeModel(GL_SMOOTH);
		break;
	case 6:
		bbox = 0;
		break;
	case 7:
		bbox = 1;
		break;
	case 8:
		collision = true;
		bbox = 1;
		break;
	case 9:
		collision = false;
		bbox = 0;
		break;
	case 10:
		//view_mode = 0;
		break;
	case 11:
		center_x = car_x;
		center_y = car_y + 1;
		center_z = car_z;
		camera_x = center_x - sin(car_angle * PI / 180);
		camera_y = center_y;
		camera_z = center_z - cos(car_angle * PI / 180);
		up_x = 0.0;
		up_y = 1.0;
		up_z = 0.0;
		view_mode = 1;
		break;
	case 12:
		navigation_mode = 0;
		break;
	case 13:
		navigation_mode = 1;
		break;
	case 14:
		material_mode = 1;
		break;
	case 15:
		material_mode = 2;
		break;
	case 16:
		material_mode = 0;
		break;
	case 17:
		normal_mode = 1;
		createDisplayList();
		break;
	case 18:
		normal_mode = 0;
		createDisplayList();
		break;
	case 19:
		normal_mode = 2;
		createDisplayList();
		break;
	case 20:
		visualization = true;
		createDisplayList();
		break;
	case 21:
		visualization = false;
		createDisplayList();
		break;
	case 22:
		plane_mode = 0;
		changePlane();
		break;
	case 23:
		plane_mode = 1;
		changePlane();
		break;
	case 24:
		plane_mode = 2;
		changePlane();
		break;
	case 25:
		texture_mode = true;
		createDisplayList();
		break;
	case 26:
		texture_mode = false;
		createDisplayList();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void glutMenus(void) {
	int view = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Top Down", 0);
	//glutAddMenuEntry("45 degree", 1);
	//glutAddMenuEntry("Overview", 10);
	glutAddMenuEntry("Overview", 1);
	glutAddMenuEntry("First Person", 11);
	int displayList = glutCreateMenu(menuEvent);
	glutAddMenuEntry("ON", 2);
	glutAddMenuEntry("OFF", 3);
	int shading = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Flat", 4);
	glutAddMenuEntry("Smooth", 5);
	int bounding_box = glutCreateMenu(menuEvent);
	glutAddMenuEntry("OFF", 6);
	glutAddMenuEntry("Axis Aligned", 7);
	int collision = glutCreateMenu(menuEvent);
	glutAddMenuEntry("ON", 8);
	glutAddMenuEntry("OFF", 9);
	int navigation = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Light Source", 12);
	glutAddMenuEntry("Camera", 13);
	int material = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Diffuse", 14);
	glutAddMenuEntry("Specular", 15);
	glutAddMenuEntry("Design", 16);
	int normal = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Per Face Normals", 17);
	glutAddMenuEntry("Per Vertex Normals", 18);
	glutAddMenuEntry("Per Vertex Normals Weighted", 19);
	int visualization = glutCreateMenu(menuEvent);
	glutAddMenuEntry("On", 20);
	glutAddMenuEntry("Off", 21);
	int plane = glutCreateMenu(menuEvent);
	glutAddMenuEntry("Flat", 22);
	glutAddMenuEntry("Multiscale", 23);
	glutAddMenuEntry("Marble", 24);
	int texture = glutCreateMenu(menuEvent);
	glutAddMenuEntry("On", 25);
	glutAddMenuEntry("Off", 26);


	int main_menu = glutCreateMenu(menuEvent);
	glutAddSubMenu("View", view);
	glutAddSubMenu("Texture", texture);
	glutAddSubMenu("Display List", displayList);
	glutAddSubMenu("Shading", shading);
	glutAddSubMenu("Bounding Box", bounding_box);
	glutAddSubMenu("Collision", collision);
	glutAddSubMenu("Navigation", navigation);
	glutAddSubMenu("Material", material);
	glutAddSubMenu("Normals", normal);
	glutAddSubMenu("Visualization", visualization);
	glutAddSubMenu("Plane", plane);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void renderBitmapString(float x, float y, float z, char *string) {
	char *c;
	glRasterPos3f(x, y, z);
	for (c = string; *c != '\0'; c++)
		glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
}

// stroke
void renderStrokeString(float x, float y, float z, float scale, void *font, char *string) {
	char *c;
	glPushMatrix();
	glTranslatef(x, y, z);   // fonts position
	glScalef(scale * 2, scale * 10, scale);
	for (c = string; *c != '\0'; c++)
		glutStrokeCharacter(font, *c);
	glPopMatrix();
}

void createDisplayList() {
	wall_DL = meshToDisplayList(wall, 1);
	car_DL = meshToDisplayList(car_model, 0);
	lego_DL = meshToDisplayList(lego_model, 0);
	plane_DL = meshToDisplayList(plane, 2);
	sky_DL = meshToDisplayList(sky, 3);
	cylinder_DL = meshToDisplayList(cylinder, 4);
}

void init() {
	time(&start_time);
	// use black to clean background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// always re-normalize normal (due to scale transform)
	glEnable(GL_NORMALIZE);
	// LIGHT

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	
	// smooth shading
	glShadeModel(GL_SMOOTH);

	// load model
	wall = create_wall(0, 0, 0);
	car_model = load_car(modelfile[3]);
	lego_model = load_lego(modelfile[6]);
	plane = create_plane(500, 500, 10);
	sky = createSky(700, 1400);
	cylinder = createCylinder(1, 6);

	
	//cout << "car: " << car_model->face_index_vertex.size() << endl;
	//cout << "lego: " << lego_model->face_index_vertex.size() << endl;
	//cout << "plane: " << plane->face_index_vertex.size() << endl;
	//cout << "plane vertex: " << plane->dot_vertex.size() << endl;
	//cout << "cylinder: " << cylinder->face_index_vertex.size() << endl;


	calculateNormalPerFace(wall);
	calculateNormalPerFace(plane);
	calculateNormalPerFace(car_model);
	calculateNormalPerFace(lego_model);
	calculateNormalPerFace(sky);
	calculateNormalPerFace(cylinder);

	calculateNormalPerVertex(wall);
	calculateNormalPerVertex(plane);
	calculateNormalPerVertex(car_model);
	calculateNormalPerVertex(lego_model);
	calculateNormalPerVertex(sky);
	calculateNormalPerVertex(cylinder);

	calculateNormalPerVertexWeighted(wall);
	calculateNormalPerVertexWeighted(plane);
	calculateNormalPerVertexWeighted(car_model);
	calculateNormalPerVertexWeighted(lego_model);
	calculateNormalPerVertexWeighted(sky);
	calculateNormalPerVertexWeighted(cylinder);

	/*
	calculateNormalWithCrease(wall, 30);
	calculateNormalWithCrease(plane, 30);
	calculateNormalWithCrease(car_model, 30);
	calculateNormalWithCrease(lego_model, 30);
	calculateNormalWithCrease(sky, 30);
	calculateNormalWithCrease(cylinder, 30);
	*/

	bmpTexture(textureArray, "../bmp files/wall.bmp", 0);
	bmpTexture(textureArray, "../bmp files/landscape.bmp", 1);

	codedTexture(textureArray, 2, 0);
	codedTexture(textureArray, 3, 1);

	createDisplayList();

	initialize_lego_models();
	calculate_lego_models_aabbox();

	// setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	aspect = (double)width / (double)height;
	gluPerspective(fieldofview, aspect, nearPlane, farPlane);
	// setup viewing matrix
	center_x = 10.0;
	center_y = 5;
	center_z = -5;
	degree_around_terrain = 0;
	radius_around_terrain = 30;
	camera_x = center_x + radius_around_terrain * sin(degree_around_terrain * PI / 180);
	camera_y = center_y + 20;
	camera_z = center_z + radius_around_terrain * cos(degree_around_terrain * PI / 180);
	up_x = 0.0;
	up_y = 1.0;
	up_z = 0.0;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_x, camera_y, camera_z,
		center_x, center_y, center_z,
		up_x, up_y, up_z);

	// set up menus
	glutMenus();
}

void testView() {
	// projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluPerspective(45, aspect, 1, 1000);
	// view
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	// lookAt
	gluLookAt(0.0f, 20.0f, 20.0,
		0.0f, 1.0f, -1.0f,
		0.0f, 1.0f, 0.0f);
	// camera
	glScalef(scale, scale, scale);
	glRotatef(x_angle, 1.0f, 0.0f, 0.0f);
	glRotatef(y_angle, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, 0.0f);
}

void material_diffuse() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
	GLfloat mat_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
	GLfloat mat_diffuse0[] = { 1, 1, 1, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
	GLfloat mat_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);
}

void material_specular() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
	GLfloat mat_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
	GLfloat mat_diffuse0[] = { 1, 1, 1, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
	GLfloat mat_specular0[] = { 1, 1, 1, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);
}

void materialForPlane() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat mat_diffuse0[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 12.8);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
	
}

void materialForSky() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.52, 0.80, 0.92, 1.0 };
	GLfloat mat_diffuse0[] = { 0.52, 0.80, 0.92, 1.0 };
	GLfloat mat_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 51.2);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
}

void materialForWall() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.2125, 0.1275, 0.054, 1.0 };
	GLfloat mat_diffuse0[] = { 0.714, 0.4284, 0.18144, 1.0 };
	GLfloat mat_specular0[] = { 0.393548, 0.271906, 0.166721, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 25.6);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
}

void materialForCar() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.25, 0.20725, 0.20725, 1.0 };
	GLfloat mat_diffuse0[] = { 1, 0.829, 0.829, 1.0 };
	GLfloat mat_specular0[] = { 0.296648, 0.296648, 0.296648, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 11.264
		);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
}

void materialForCylinder() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.19225, 0.19225, 0.19225, 1.0 };
	GLfloat mat_diffuse0[] = { 0.50754, 0.50754, 0.50754, 1.0 };
	GLfloat mat_specular0[] = { 1, 1, 1, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 51.2);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
}

void materialForLego() {
	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient0[] = { 0.0215, 0.1745, 0.0215, 1.0 };
	GLfloat mat_diffuse0[] = { 0.07568, 0.61424, 0.07568, 1.0 };
	GLfloat mat_specular0[] = { 0.633, 0.727811, 0.633, 1.0 };
	switch (material_mode)
	{
	case 0: // design
		glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
		glMaterialf(GL_FRONT, GL_SHININESS, 76.8);
		break;
	case 1: // diffuse
		material_diffuse();
		break;
	case 2: // specular
		material_specular();
		break;
	default:
		break;
	}
}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(camera_x, camera_y, camera_z,
		center_x, center_y, center_z,
		up_x, up_y, up_z);

	glPushMatrix();
	glRotatef(light_degree, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glPopMatrix();
	
	//testView();

	glViewport(position_x, position_y + height * 0.25, width, height * 0.75);

	// draw plane
	materialForPlane();
	draw_plane();

	// draw sky

	materialForSky();
	glPushMatrix();
	glTranslatef(-50, -200, 0);
	glCallList(sky_DL);
	glPopMatrix();

	

	// draw cylinder
	materialForCylinder();
	glPushMatrix();
	glTranslatef(-5, -2, 0);
	glCallList(cylinder_DL);
	glPopMatrix();

	// draw checkpoints
	materialForWall();
	draw_checkpoints();

	// draw lybrinth
	materialForWall();
	draw_all_walls();

	// draw car model
	materialForCar();
	draw_car();

	// draw lego models
	materialForLego();
	draw_lego();

	// draw aabbox
	materialForWall();
	if (bbox == 1) {
		draw_all_aabboxes();
	}

	GLfloat mat_emission0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
	GLfloat mat_ambient0[] = { 0.2, 0.2, 0.2, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
	GLfloat mat_diffuse0[] = { 0.8, 0.8, 0.8, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
	GLfloat mat_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	glViewport(position_x, position_y, width, height * 0.25);

	if (!isStart) {
		time(&start_time);
		current_time = start_time;
	}
	else {
		if (!isFinish) {
			time(&current_time);
		}
	}

	int second = difftime(current_time, start_time);
	string s = to_string(second);
	char text[1024];
	strcpy(text, s.c_str());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(1.0, 0, 0);
	GLfloat mat_emission2[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission2);
	GLfloat mat_ambient2[] = { 0.52, 0.80, 0.92, 1.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient2);
	GLfloat mat_diffuse2[] = { 0.52, 0.80, 0.92, 1.0 };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse2);
	GLfloat mat_specular2[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular2);
	glMaterialf(GL_FRONT, GL_SHININESS, 51.2);

	renderStrokeString(position_x, position_y + height * 0.5, 0.0f, 0.1, GLUT_STROKE_ROMAN, "Elapsed Time: ");
	renderStrokeString(position_x + 200, position_y + height * 0.5, 0.0f, 0.1, GLUT_STROKE_ROMAN, text);
	if (isStart && !isFinish) {
		renderStrokeString(position_x + 500, position_y + height * 0.5, 0.0f, 0.1, GLUT_STROKE_ROMAN, "Game Start!");
	}
	if (isFinish) {
		renderStrokeString(position_x + 500, position_y + height * 0.5, 0.0f, 0.1, GLUT_STROKE_ROMAN, "Game Over!");
	}

	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse0);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular0);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glutSwapBuffers();

	// move lego models
	if (!isFinish) {
		move_all_lego_models();
	}
}

void mouse(int button, int state, int x, int y) {
	mouse_x = x;
	mouse_y = y;
	mouse_button = button;
}

void motion(int x, int y) {
	if (mouse_button == GLUT_LEFT_BUTTON) {
		y_angle += (float(x - mouse_x) / width) *360.0;
		x_angle += (float(y - mouse_y) / height)*360.0;
	}
	if (mouse_button == GLUT_RIGHT_BUTTON) {
		scale += (y - mouse_y) / 100.0;
		if (scale < 0.1) scale = 0.1;
		if (scale > 7)	scale = 7;
	}
	mouse_x = x;
	mouse_y = y;
	glutPostRedisplay();
}

void reshape(int x, int y) {
	width = x;
	height = y;
	if (height == 0)		// not divided by zero
		height = 1;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	aspect = (float)width / (float)height;
	gluPerspective(fieldofview, aspect, nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
}

void move_single_lego_model(unsigned i, float angle, float radius) {
	glm::mat4 myMatrix;
	lego_parameters pre_param = lego_param[i];
	AABBox pre_aabbox = lego_aabboxes[i];

	lego_param[i].lego_angle = lego_param[i].lego_angle + angle;
	if (lego_param[i].lego_angle > 360) lego_param[i].lego_angle -= 360;
	lego_param[i].lego_coordinates.x = lego_param[i - 1].lego_coordinates.x + radius * sin(lego_param[i].lego_angle * PI / 180);
	lego_param[i].lego_coordinates.z = lego_param[i - 1].lego_coordinates.z + radius * cos(lego_param[i].lego_angle * PI / 180);

	lego_aabboxes[i] = lego_orig_aabbox;
	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(lego_param[i].lego_coordinates.x, lego_param[i].lego_coordinates.y, lego_param[i].lego_coordinates.z)) *
		glm::rotate(glm::mat4(1.0f), (float)(lego_param[i].lego_angle * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(lego_scale, lego_scale, lego_scale));

	transform_aabbox(myMatrix, lego_aabboxes[i].min_v, lego_aabboxes[i].max_v);

	if (collision && (lego_collision_detection_with_car(i) || lego_collision_with_each_other(i))) {
		lego_param[i] = pre_param;
		lego_aabboxes[i] = pre_aabbox;
	}
}

void move_all_lego_models() {
	move_single_lego_model(1, 1, 3.5);
	move_single_lego_model(2, 5, 1.5);
	move_single_lego_model(4, 5, 1.5);

	glutPostRedisplay();
}

void moveCarFlat(float i) {

	car_x += i * sin(car_angle*PI / 180);
	car_z += i * cos(car_angle*PI / 180);

}

void inputKey(unsigned char c) {
	if (isFinish) {
		return;
	}

	float fx = car_x;
	float fy = car_y;
	float fz = car_z;
	float fangle = car_angle;
	glm::mat4 myMatrix;
	car_aabbox = car_orig_aabbox;

	switch (c) {
	case 'a':
		car_angle += 10;
		break;
	case 'd':
		car_angle -= 10;
		break;
	case 'w':
		moveCarFlat(1);
		break;
	case 's':
		moveCarFlat(-1);
		break;
	}

	myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car_x, car_y, car_z)) *
		glm::rotate(glm::mat4(1.0f), (float)(car_angle * PI / 180), glm::vec3(0, 1, 0)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(car_scale, car_scale, car_scale));

	transform_aabbox(myMatrix, car_aabbox.min_v, car_aabbox.max_v);

	if (collision && car_collision_detection()) {
		car_aabbox = car_orig_aabbox;
		car_x = fx;
		car_y = fy;
		car_z = fz;
		car_angle = fangle;
		myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(car_x, car_y, car_z)) *
			glm::rotate(glm::mat4(1.0f), (float)(car_angle * PI / 180), glm::vec3(0, 1, 0)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(car_scale, car_scale, car_scale));

		transform_aabbox(myMatrix, car_aabbox.min_v, car_aabbox.max_v);
	}

	if (!isStart) {
		if (is_start()) {
			isStart = true;
		}
	}

	if (!isFinish) {
		if (is_finish()) {
			isFinish = true;
		}
	}

	glutPostRedisplay();
}

void navigation_light(int degree) {
	light_degree = (light_degree + degree) % 360;
}

void navigation_aroung_terrain(int degree, int y) {
	degree_around_terrain = (degree_around_terrain + degree) % 360;
	terrain_move_y += y;
	camera_x = center_x + radius_around_terrain * sin(degree_around_terrain * PI / 180);
	camera_y = center_y + 20 + terrain_move_y;
	camera_z = center_z + radius_around_terrain * cos(degree_around_terrain * PI / 180);
}

void navigation_first_person(int operation, int y) {
	switch (operation) {
	case 0: // forward
		inputKey('w');
		break;
	case 1: // back
		inputKey('s');
		break;
	case 2: // left
		inputKey('a');
		break;
	case 3: // right
		inputKey('d');
		break;
	case 4: // a
		car_y += y;
		break;
	case 5: // s
		car_y += y;
		break;
	}
	center_x = car_x;
	center_y = car_y + 1;
	center_z = car_z;
	camera_x = center_x - sin(car_angle * PI / 180);
	camera_y = center_y;
	camera_z = center_z - cos(car_angle * PI / 180);
}

void controller(unsigned char c, int x, int y) {
	switch (c) {
	case 'a':
		if (view_mode == 0 && navigation_mode == 1) {
			radius_around_terrain += 1;
			navigation_aroung_terrain(0, 0);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(4, +1);
		}
		break;
	case 'd':
		break;
	case 'w':
		break;
	case 's':
		if (view_mode == 0 && navigation_mode == 1) {
			if (radius_around_terrain > 3) {
				radius_around_terrain -= 1;
			}
			navigation_aroung_terrain(0, 0);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(5, -1);
		}
		break;
	}

	glutPostRedisplay();

}

void specialKeys(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_UP:
		if (view_mode == 0 && navigation_mode == 1) {
			navigation_aroung_terrain(0, +1);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(0, 0);
		}
		break;
	case GLUT_KEY_DOWN:
		if (view_mode == 0 && navigation_mode == 1) {
			navigation_aroung_terrain(0, -1);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(1, 0);
		}
		break;
	case GLUT_KEY_LEFT:
		if (navigation_mode == 0) {
			navigation_light(-10);
		}
		if (view_mode == 0 && navigation_mode == 1) {
			navigation_aroung_terrain(-10, 0);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(2, 0);
		}
		break;
	case GLUT_KEY_RIGHT:
		if (navigation_mode == 0) {
			navigation_light(+10);
		}	
		if (view_mode == 0 && navigation_mode == 1) {
			navigation_aroung_terrain(+10, 0);
		}
		if (view_mode == 1 && navigation_mode == 1) {
			navigation_first_person(3, 0);
		}
		break;
	}
	glutPostRedisplay();
}

void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(position_x, position_y);
	glutInitWindowSize(width, height);
	winId = glutCreateWindow("Project_2");
	glutReshapeFunc(reshape);
	//glutKeyboardFunc(inputKey);
	glutKeyboardFunc(controller);
	glutSpecialFunc(specialKeys);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutDisplayFunc(display);		// display function
	init();
	glutMainLoop();
}