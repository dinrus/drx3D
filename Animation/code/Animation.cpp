#include <drx3D/Animation/Animation.h>

namespace drx3d {
Animation::Animation(const Time &length, std::vector<Keyframe> keyframes) :
	length(length),
	keyframes(std::move(keyframes)) {
}

const Node &operator>>(const Node &node, Animation &animation) {
	node["length"].Get(animation.length);
	node["keyframes"].Get(animation.keyframes);
	return node;
}

Node &operator<<(Node &node, const Animation &animation) {
	node["length"].Set(animation.length);
	node["keyframes"].Set(animation.keyframes);
	return node;
}
}
