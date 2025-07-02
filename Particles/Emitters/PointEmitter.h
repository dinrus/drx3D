#pragma once

#include <drx3D/Particles/Emitters/Emitter.h>

namespace drx3d {
class DRX3D_EXPORT PointEmitter : public Emitter::Registry<PointEmitter> {
	inline static const bool Registered = Register("point");
public:
	PointEmitter();

	Vector3f GeneratePosition() const override;

	const Vector3f &GetPoint() const { return point; }
	void SetPoint(const Vector3f &point) { this->point = point; }

	friend const Node &operator>>(const Node &node, PointEmitter &emitter);
	friend Node &operator<<(Node &node, const PointEmitter &emitter);

private:
	Vector3f point;
};
}
