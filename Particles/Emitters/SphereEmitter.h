#pragma once

#include <drx3D/Particles/Emitters/Emitter.h>

namespace drx3d {
class DRX3D_EXPORT SphereEmitter : public Emitter::Registry<SphereEmitter> {
	inline static const bool Registered = Register("sphere");
public:
	explicit SphereEmitter(float radius = 1.0f);

	Vector3f GeneratePosition() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius) { this->radius = radius; }

	friend const Node &operator>>(const Node &node, SphereEmitter &emitter);
	friend Node &operator<<(Node &node, const SphereEmitter &emitter);

private:
	float radius;
};
}
