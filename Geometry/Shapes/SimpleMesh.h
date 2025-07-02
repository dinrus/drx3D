#pragma once

#include <drx3D/Geometry/Model.h>
#include <drx3D/Geometry/Vertex3d.h>

namespace drx3d {
class DRX3D_EXPORT SimpleMesh : public Model {
public:
	SimpleMesh(float sideLength, float squareSize, uint32_t vertexCount, float uvScale);

protected:
	virtual Vertex3d GetVertex(uint32_t col, uint32_t row);

	void GenerateMesh();

	float sideLength;
	float squareSize;
	uint32_t vertexCount;
	float uvScale;
};
}
