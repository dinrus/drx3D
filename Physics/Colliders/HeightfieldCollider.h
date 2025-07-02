#pragma once

#include <drx3D/Physics/Colliders/Collider.h>

class HeightfieldTerrainShape;

namespace drx3d {
class DRX3D_EXPORT HeightfieldCollider : public Collider::Registry<HeightfieldCollider> {
	inline static const bool Registered = Register("heightfield");
public:
	explicit HeightfieldCollider(int32_t heightStickWidth = 100, int32_t heightStickLength = 100, ukk heightfieldData = nullptr,
		float minHeight = -1.0f, float maxHeight = 1.0f, bool flipQuadEdges = false, const Transform &localTransform = {});
	~HeightfieldCollider();

	CollisionShape *GetCollisionShape() const override;

	void SetHeightfield(int32_t heightStickWidth, int32_t heightStickLength, ukk heightfieldData, float minHeight, float maxHeight,
		bool flipQuadEdges);

	friend const Node &operator>>(const Node &node, HeightfieldCollider &collider);
	friend Node &operator<<(Node &node, const HeightfieldCollider &collider);

private:
	std::unique_ptr<HeightfieldTerrainShape> shape;
};
}
