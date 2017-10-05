#pragma once
#include <stdlib.h>
#include "glut.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MIN2(a,b) (((a) < (b))?(a):(b))
#define MAX2(a,b) (((a) > (b))?(a):(b))

struct AABBox {
	Vec3f min_v;
	Vec3f max_v;
	Mesh* mesh;
};

void calc_aabbox(Mesh* model, Vec3f& bbxmin, Vec3f& bbxmax);
void transform_aabbox(glm::mat4 myMatrix, Vec3f& bbxmin, Vec3f& bbxmax);
void recalculate_aabbox(vector<Vec3f> bbox_vertices, Vec3f& bbxmin, Vec3f& bbxmax);
void draw_aabbox(AABBox aabbox);
#pragma once
