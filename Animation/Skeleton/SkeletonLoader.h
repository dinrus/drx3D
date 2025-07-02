#pragma once

#include <drx3D/Files/Node.h>
#include <drx3D/Animation/Skeleton/Joint.h>

namespace drx3d {
class DRX3D_EXPORT SkeletonLoader {
public:
	SkeletonLoader(NodeConstView &&libraryControllers, std::vector<STxt> boneOrder, const Matrix4 &correction);

	uint32_t GetJointCount() const { return jointCount; }
	const Joint &GetHeadJoint() const { return headJoint; }

private:
	Joint LoadJointData(const Node &jointNode, bool isRoot);
	Joint ExtractMainJointData(const Node &jointNode, bool isRoot);
	std::optional<uint32_t> GetBoneIndex(const STxt &name) const;

	NodeConstView armatureData;
	std::vector<STxt> boneOrder;
	Matrix4 correction;

	uint32_t jointCount = 0;
	Joint headJoint;
};
}
