#pragma once

#include <drx3D/Maths/Matrix4.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Animation/Skin/VertexWeights.h>
#include <drx3D/Animation/Geometry/VertexAnimated.h>

namespace drx3d {
class DRX3D_EXPORT GeometryLoader : NonCopyable {
public:
	GeometryLoader(NodeConstView &&libraryGeometries, std::vector<VertexWeights> vertexWeights, const Matrix4 &correction);

	const std::vector<VertexAnimated> &GetVertices() const { return vertices; }
	const std::vector<uint32_t> &GetIndices() const { return indices; }

private:
	std::vector<Vector3f> GetPositions() const;
	std::vector<Vector2f> GetUvs() const;
	std::vector<Vector3f> GetNormals() const;

	NodeConstView meshData;

	std::vector<VertexWeights> vertexWeights;
	Matrix4 correction;

	std::vector<VertexAnimated> vertices;
	std::vector<uint32_t> indices;
};
}
