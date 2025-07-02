#include <drx3D/Animation/Keyframe.h>

namespace drx3d {
Keyframe::Keyframe(const Time &timeStamp, std::map<STxt, JointTransform> pose) :
	timeStamp(timeStamp),
	pose(std::move(pose)) {
}

void Keyframe::AddJointTransform(const STxt &jointNameId, const Matrix4 &jointLocalTransform) {
	pose.emplace(jointNameId, jointLocalTransform);
}

const Node &operator>>(const Node &node, Keyframe &keyframe) {
	node["timeStamp"].Get(keyframe.timeStamp);
	node["pose"].Get(keyframe.pose);
	return node;
}

Node &operator<<(Node &node, const Keyframe &keyframe) {
	node["timeStamp"].Set(keyframe.timeStamp);
	node["pose"].Set(keyframe.pose);
	return node;
}
}
