#pragma once

#include <drx3D/Particles/Emitters/Emitter.h>

namespace drx3d {
class DRX3D_EXPORT CircleEmitter : public Emitter::Registry<CircleEmitter> {
	inline static const bool Registered = Register("circle");
public:
	explicit CircleEmitter(float radius = 1.0f, const Vector3f &heading = Vector3f::Up);

	Vector3f GeneratePosition() const override;

	float GetRadius() const { return radius; }
	void SetRadius(float radius) { this->radius = radius; }

	const Vector3f &GetHeading() const { return heading; }
	void SetHeading(const Vector3f &heading) { this->heading = heading; }

	friend const Node &operator>>(const Node &node, CircleEmitter &emitter);
	friend Node &operator<<(Node &node, const CircleEmitter &emitter);

private:
	float radius;
	Vector3f heading;
};
}
