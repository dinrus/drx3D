#include <drx3D/Particles/Emitters/PointEmitter.h>

#include <drx3D/Scenes/Entity.h>

namespace drx3d {
PointEmitter::PointEmitter() {
}

Vector3f PointEmitter::GeneratePosition() const {
	return point;
}

const Node &operator>>(const Node &node, PointEmitter &emitter) {
	node["point"].Get(emitter.point);
	return node;
}

Node &operator<<(Node &node, const PointEmitter &emitter) {
	node["point"].Set(emitter.point);
	return node;
}
}
