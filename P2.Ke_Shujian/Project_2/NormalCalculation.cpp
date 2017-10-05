#include "NormalCalculation.h"

float calcTriangleArea(const Vec3<float> &v1, const Vec3<float> &v2, const Vec3<float> &v3) {
	Vec3<float> AB = v2 - v1, AC = v3 - v1;
	return (AB.cross(AC).length()) / 2;
}

float calculateCreaseAngle(Vec3f n1, Vec3f n2) {
	float dot_product = n1.dot(n2);
	float cosValue = dot_product / (n1.length() * n2.length());
	float angle = acos(cosValue) * 180 / 3.1415926;
	return angle;
}

// normal per face
void calculateNormalPerFace(Mesh* m) {
	Vec3<float> v1, v2, v3, v4, v5;
	for (int i = 0; i < m->face_index_vertex.size(); i += 3) {
		v1 = m->dot_vertex[m->face_index_vertex[i]];
		v2 = m->dot_vertex[m->face_index_vertex[i + 1]];
		v3 = m->dot_vertex[m->face_index_vertex[i + 2]];
		v4 = (v2 - v1);
		v5 = (v3 - v1);
		v4 = v4.cross(v5);
		v4.normalize();
		m->dot_normalPerFace.push_back(v4);
		int pos = m->dot_normalPerFace.size() - 1;
		// same normal for all vertex in this face
		m->face_index_normalPerFace.push_back(pos);
		m->face_index_normalPerFace.push_back(pos);
		m->face_index_normalPerFace.push_back(pos);
	}
}

// calculate normal per vertex
void calculateNormalPerVertex(Mesh* m) {
	m->dot_normalPerVertex.clear();
	m->face_index_normalPerVertex.clear();
	Vec3<float> suma; suma.x = 0; suma.y = 0; suma.z = 0;
	//initialize
	for (unsigned int val = 0; val < m->dot_vertex.size(); val++) {
		m->dot_normalPerVertex.push_back(suma);
	}
	// calculate sum for vertex
	for (long pos = 0; pos < m->face_index_vertex.size(); pos++) {
		m->dot_normalPerVertex[m->face_index_vertex[pos]] +=
			m->dot_normalPerFace[m->face_index_normalPerFace[pos]];
	}
	// normalize for vertex 
	for (unsigned int val = 0; val < m->dot_normalPerVertex.size(); val++) {
		m->dot_normalPerVertex[val] = m->dot_normalPerVertex[val].normalize();
	}
	//normalVertexIndex is the same that vertexIndex
	for (unsigned int pos = 0; pos < m->face_index_vertex.size(); pos++) {
		m->face_index_normalPerVertex.push_back(m->face_index_vertex[pos]);
	}
}

// calculate normal per vertex weighted
void calculateNormalPerVertexWeighted(Mesh* m) {
	m->dot_normalPerVertexWeighted.clear();
	m->face_index_normalPerVertexWeighted.clear();
	Vec3<float> suma; suma.x = 0; suma.y = 0; suma.z = 0;
	//initialize
	for (unsigned int val = 0; val < m->dot_vertex.size(); val++) {
		m->dot_normalPerVertexWeighted.push_back(suma);
	}
	// calculate sum for vertex
	float areaOfTriangle = 0;
	for (long pos = 0; pos < m->face_index_vertex.size(); pos++) {
		if (pos % 3 == 0) {
			areaOfTriangle = calcTriangleArea(m->dot_vertex[m->face_index_vertex[pos]],
				m->dot_vertex[m->face_index_vertex[pos + 1]],
				m->dot_vertex[m->face_index_vertex[pos + 2]]);
		}
		m->dot_normalPerVertexWeighted[m->face_index_vertex[pos]] +=
			m->dot_normalPerFace[m->face_index_normalPerFace[pos]] * areaOfTriangle;
	}
	// normalize for vertex 
	for (unsigned int val = 0; val < m->dot_normalPerVertexWeighted.size(); val++) {
		m->dot_normalPerVertexWeighted[val] = m->dot_normalPerVertexWeighted[val].normalize();
	}
	//normalVertexIndex is the same that vertexIndex
	for (unsigned int pos = 0; pos < m->face_index_vertex.size(); pos++) {
		m->face_index_normalPerVertexWeighted.push_back(m->face_index_vertex[pos]);
	}
}


// this method is not efficient, the time compelxity is n^2.  
void calculateNormalWithCrease(Mesh* m, float creaseAngle) {
	m->dot_normalPerVertexWeightedWithCrease.clear();
	m->face_index_normalPerVertexWeightedWithCrese.clear();

	for (int i = 0; i < m->face_index_vertex.size(); i += 1) {
		Vec3f sum = Vec3f(0, 0, 0);
		Vec3f n1 = m->dot_normalPerFace[m->face_index_normalPerFace[i]];
		sum += m->dot_normalPerFace[m->face_index_normalPerFace[i]];
		for (int j = 0; j < m->face_index_vertex.size(); j += 1) {
			if (i == j) continue;
			if (m->face_index_vertex[i] != m->face_index_vertex[j]) continue;
			Vec3f n2 = m->dot_normalPerFace[m->face_index_normalPerFace[j]];
			//cout << "i: " << i << " normal: " << n1 << endl;
			//cout << "j: " << j << " normal: " << n2 << endl;
			//cout << "angle: " << calculateCreaseAngle(n1, n2) << endl;
			if (calculateCreaseAngle(n1, n2) > creaseAngle) continue;
			sum += n2;
		}
		sum.normalize();
		//cout << "i: " << i << " final normal: " << sum << endl;
		m->dot_normalPerVertexWeightedWithCrease.push_back(sum);
		int pos = m->dot_normalPerVertexWeightedWithCrease.size() - 1;
		m->face_index_normalPerVertexWeightedWithCrese.push_back(pos);
	}

}