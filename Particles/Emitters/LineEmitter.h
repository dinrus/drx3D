#pragma once

#include <drx3D/Particles/Emitters/Emitter.h>

namespace drx3d {
class DRX3D_EXPORT LineEmitter : public Emitter::Registry<LineEmitter> {
	inline static const bool Registered = Register("line");
public:
	explicit LineEmitter(float length = 1.0f, const Vector3f &axis = Vector3f::Right);

	Vector3f GeneratePosition() const override;

	float GetLength() const { return length; }
	void SetLength(float length) { this->length = length; }

	const Vector3f &GetAxis() const { return axis; }
	void SetAxis(const Vector3f &axis) { this->axis = axis; }

	friend const Node &operator>>(const Node &node, LineEmitter &emitter);
	friend Node &operator<<(Node &node, const LineEmitter &emitter);

private:
	float length;
	Vector3f axis;
};
}
