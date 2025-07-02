#include <drx3D/Animation/AnimationLoader.h>

#include <drx3D/Animation/AnimatedMesh.h>
#include <drx3D/Common/Enumerate.h>

namespace drx3d {
AnimationLoader::AnimationLoader(NodeConstView &&libraryAnima, NodeConstView &&libraryVisualScenes, const Matrix4 &correction) :
	libraryAnima(std::move(libraryAnima)),
	libraryVisualScenes(std::move(libraryVisualScenes)),
	correction(correction) {
	auto animationNodes = libraryAnima["animation"];

	auto rootNode = FindRootJointName();
	auto times = GetKeyTimes();
	lengthSeconds = times[times.size() - 1];
	CreateKeyframe(times);

	for (const auto &[jointNodeName, jointNode]: animationNodes.GetProperties())
		LoadJointTransforms(jointNode, rootNode);
}

STxt AnimationLoader::FindRootJointName() const {
	auto skeleton = libraryVisualScenes["visual_scene"]["node"].GetPropertyWithValue("@id", "Armature");
	return skeleton["node"]["@id"].Get<STxt>();
}

std::vector<Time> AnimationLoader::GetKeyTimes() const {
	// Times should be the same for each pose so we grab the first joint times.
	auto timeData = libraryAnima["animation"][0]["source"][0]["float_array"];
	auto rawTimes = String::Split(timeData.Get<STxt>(), ' ');

	std::vector<Time> times;
	times.reserve(rawTimes.size());
	for (const auto &rawTime : rawTimes)
		times.emplace_back(Time::Seconds(String::From<float>(rawTime)));
	return times;
}

void AnimationLoader::CreateKeyframe(const std::vector<Time> &times) {
	for (const auto &time : times)
		keyframes.emplace_back(Keyframe(time, {}));
}

void AnimationLoader::LoadJointTransforms(const Node &jointData, const STxt &rootNodeId) {
	auto jointNameId = GetJointName(jointData);
	auto dataId = GetDataId(jointData);

	auto transformData = jointData["source"].GetPropertyWithValue("@id", dataId);

	auto data = transformData["float_array"].Get<STxt>();
	auto splitData = String::Split(data, ' ');
	ProcessTransforms(jointNameId, splitData, jointNameId == rootNodeId);
}

STxt AnimationLoader::GetDataId(const Node &jointData) {
	auto node = jointData["sampler"]["input"].GetPropertyWithValue("@semantic", "OUTPUT");
	return node["@source"].Get<STxt>().substr(1);
}

STxt AnimationLoader::GetJointName(const Node &jointData) {
	auto channelNode = jointData["channel"];
	auto data = channelNode["@target"].Get<STxt>();
	auto splitData = String::Split(data, '/');
	return splitData[0];
}

void AnimationLoader::ProcessTransforms(const STxt &jointName, const std::vector<STxt> &rawData, bool root) {
	for (auto [i, keyframe] : Enumerate(keyframes)) {
		Matrix4 transform;

		for (uint32_t row = 0; row < 4; row++) {
			for (uint32_t col = 0; col < 4; col++) {
				transform[row][col] = String::From<float>(rawData[16 * i + (row * 4 + col)]);
			}
		}

		transform = transform.Transpose();
		if (root)
			transform = correction * transform;

		keyframe.AddJointTransform(jointName, transform);
	}
}
}
