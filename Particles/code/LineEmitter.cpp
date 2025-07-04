#include <drx3D/Particles/Emitters/LineEmitter.h>

#include <drx3D/Maths/Maths.h>
#include <drx3D/Scenes/Entity.h>

namespace drx3d {
LineEmitter::LineEmitter(float length, const Vector3f &axis) :
	length(length),
	axis(axis.Normalize()) {
}

Vector3f LineEmitter::GeneratePosition() const {
	return axis * length * Maths::Random(-0.5f, 0.5f);
}

const Node &operator>>(const Node &node, LineEmitter &emitter) {
	node["length"].Get(emitter.length);
	node["axis"].Get(emitter.axis);
	return node;
}

Node &operator<<(Node &node, const LineEmitter &emitter) {
	node["length"].Set(emitter.length);
	node["axis"].Set(emitter.axis);
	return node;
}
}
