#include "BoundingBox.h"

void draw_aabbox(AABBox aabbox) {
	vector<Vec3f> aabbox_vertices;
	aabbox_vertices.push_back(Vec3f(aabbox.min_v.x, aabbox.min_v.y, aabbox.min_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.min_v.x, aabbox.min_v.y, aabbox.max_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.min_v.x, aabbox.max_v.y, aabbox.max_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.min_v.x, aabbox.max_v.y, aabbox.min_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.max_v.x, aabbox.min_v.y, aabbox.min_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.max_v.x, aabbox.min_v.y, aabbox.max_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.max_v.x, aabbox.max_v.y, aabbox.max_v.z));
	aabbox_vertices.push_back(Vec3f(aabbox.max_v.x, aabbox.max_v.y, aabbox.min_v.z));
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_STRIP);
	glVertex3fv(&aabbox_vertices[0].x);
	glVertex3fv(&aabbox_vertices[1].x);
	glVertex3fv(&aabbox_vertices[2].x);
	glVertex3fv(&aabbox_vertices[3].x);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3fv(&aabbox_vertices[4].x);
	glVertex3fv(&aabbox_vertices[5].x);
	glVertex3fv(&aabbox_vertices[6].x);
	glVertex3fv(&aabbox_vertices[7].x);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3fv(&aabbox_vertices[0].x);
	glVertex3fv(&aabbox_vertices[4].x);
	glVertex3fv(&aabbox_vertices[7].x);
	glVertex3fv(&aabbox_vertices[3].x);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3fv(&aabbox_vertices[6].x);
	glVertex3fv(&aabbox_vertices[2].x);
	glVertex3fv(&aabbox_vertices[1].x);
	glVertex3fv(&aabbox_vertices[5].x);
	glEnd();
}

void calc_aabbox(Mesh* model, Vec3f& bbxmin, Vec3f& bbxmax) {
	vector<Vec3f>& v = model->dot_vertex;
	if (v.empty()) return;
	bbxmax = bbxmin = v[0];
	for (unsigned int i = 1; i < v.size(); i++) {
		bbxmin.x = MIN2(bbxmin.x, v[i].x);
		bbxmin.y = MIN2(bbxmin.y, v[i].y);
		bbxmin.z = MIN2(bbxmin.z, v[i].z);
		bbxmax.x = MAX2(bbxmax.x, v[i].x);
		bbxmax.y = MAX2(bbxmax.y, v[i].y);
		bbxmax.z = MAX2(bbxmax.z, v[i].z);
	}
}

void transform_aabbox(glm::mat4 myMatrix, Vec3f& bbxmin, Vec3f& bbxmax) {
	// need to calculate new position of 8 vertices
	vector<Vec3f> aabbox_vertices;
	glm::vec4 myVector;
	glm::vec4 transformedVector;

	aabbox_vertices.push_back(Vec3f(bbxmin.x, bbxmin.y, bbxmin.z));
	aabbox_vertices.push_back(Vec3f(bbxmin.x, bbxmin.y, bbxmax.z));
	aabbox_vertices.push_back(Vec3f(bbxmin.x, bbxmax.y, bbxmax.z));
	aabbox_vertices.push_back(Vec3f(bbxmin.x, bbxmax.y, bbxmin.z));
	aabbox_vertices.push_back(Vec3f(bbxmax.x, bbxmin.y, bbxmin.z));
	aabbox_vertices.push_back(Vec3f(bbxmax.x, bbxmin.y, bbxmax.z));
	aabbox_vertices.push_back(Vec3f(bbxmax.x, bbxmax.y, bbxmax.z));
	aabbox_vertices.push_back(Vec3f(bbxmax.x, bbxmax.y, bbxmin.z));


	//std::cout << bbxmin << ',' << bbxmax << std::endl;
	for (unsigned i = 0; i < aabbox_vertices.size(); i++) {
		// get new Vector 
		myVector = glm::vec4(aabbox_vertices[i].x, aabbox_vertices[i].y, aabbox_vertices[i].z, 1.0);
		transformedVector = myMatrix * myVector;
		aabbox_vertices[i] = Vec3f(transformedVector.x, transformedVector.y, transformedVector.z);
	}

	// after rotation, need to recalculate new bounding box
	recalculate_aabbox(aabbox_vertices, bbxmin, bbxmax);
	//std::cout << bbxmin << ',' << bbxmax << std::endl;
	//std::cout << std::endl;
}

// after transformation, need to recalculate 
void recalculate_aabbox(vector<Vec3f> bbox_vertices, Vec3f& bbxmin, Vec3f& bbxmax) {
	bbxmax = bbxmin = bbox_vertices[0];
	for (unsigned int i = 1; i < bbox_vertices.size(); i++) {
		bbxmin.x = MIN2(bbxmin.x, bbox_vertices[i].x);
		bbxmin.y = MIN2(bbxmin.y, bbox_vertices[i].y);
		bbxmin.z = MIN2(bbxmin.z, bbox_vertices[i].z);
		bbxmax.x = MAX2(bbxmax.x, bbox_vertices[i].x);
		bbxmax.y = MAX2(bbxmax.y, bbox_vertices[i].y);
		bbxmax.z = MAX2(bbxmax.z, bbox_vertices[i].z);
	}
}
