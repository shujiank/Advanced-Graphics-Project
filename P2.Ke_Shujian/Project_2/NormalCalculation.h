#pragma once
#include "mesh.h"

void calculateNormalPerFace(Mesh* m);

void calculateNormalPerVertex(Mesh* m);

void calculateNormalPerVertexWeighted(Mesh* m);

void calculateNormalWithCrease(Mesh* m, float creaseAngle);