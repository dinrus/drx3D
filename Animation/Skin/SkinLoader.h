#pragma once

#include <drx3D/Files/Node.h>
#include <drx3D/Animation/Skin/VertexWeights.h>

namespace drx3d {
class DRX3D_EXPORT SkinLoader {
public:
	SkinLoader(NodeConstView &&libraryControllers, uint32_t maxWeights);

	const std::vector<STxt> &GetJointOrder() const { return jointOrder; }
	const std::vector<VertexWeights> &GetVertexWeights() const { return vertexWeights; }

private:
	void LoadJointsList();
	std::vector<float> LoadWeights() const;
	std::vector<uint32_t> GetEffectiveJointsCounts(const Node &weightsDataNode) const;
	void GetSkinWeights(const Node &weightsDataNode, const std::vector<uint32_t> &counts, const std::vector<float> &weights);

	NodeConstView skinData;
	uint32_t maxWeights;

	std::vector<STxt> jointOrder;
	std::vector<VertexWeights> vertexWeights;
};
}
