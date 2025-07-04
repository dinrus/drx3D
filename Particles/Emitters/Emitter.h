#pragma once

#include <drx3D/Common/StreamFactory.h>
#include <drx3D/Maths/Vector3.h>

namespace drx3d {
/**
 * @brief Component interface that defines a emitter volume.
 */
class DRX3D_EXPORT Emitter : public StreamFactory<Emitter> {
public:
	virtual ~Emitter() = default;

	/**
	 * Creates a new objects position.
	 * @return The new objects position.
	 */
	virtual Vector3f GeneratePosition() const = 0;

	static Vector3f RandomUnitVector() {
		auto theta = Maths::Random(0.0f, 1.0f) * 2.0f * Maths::PI<float>;
		auto z = Maths::Random(0.0f, 1.0f) * 2.0f - 1.0f;
		auto rootOneMinusZSquared = std::sqrt(1.0f - z * z);
		auto x = rootOneMinusZSquared * std::cos(theta);
		auto y = rootOneMinusZSquared * std::sin(theta);
		return {x, y, z};
	}
};
}
