#pragma once

#include <drx3D/Maths/Matrix4.h>
#include <drx3D/Files/Node.h>
#include <drx3D/Animation/Animation.h>

namespace drx3d {
class DRX3D_EXPORT AnimationLoader {
public:
	AnimationLoader(NodeConstView &&libraryAnima, NodeConstView &&libraryVisualScenes, const Matrix4 &correction);

	const Time &GetLengthSeconds() const { return lengthSeconds; }
	const std::vector<Keyframe> &GetKeyframes() const { return keyframes; }

private:
	STxt FindRootJointName() const;
	std::vector<Time> GetKeyTimes() const;

	void CreateKeyframe(const std::vector<Time> &times);
	void LoadJointTransforms(const Node &jointData, const STxt &rootNodeId);

	static STxt GetDataId(const Node &jointData);
	static STxt GetJointName(const Node &jointData);

	void ProcessTransforms(const STxt &jointName, const std::vector<STxt> &rawData, bool root);

	NodeConstView libraryAnima;
	NodeConstView libraryVisualScenes;
	Matrix4 correction;

	Time lengthSeconds;
	std::vector<Keyframe> keyframes;
};
}
