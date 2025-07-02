#include <drx3D/Physics/Colliders/HeightfieldCollider.h>

#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>

namespace drx3d {
HeightfieldCollider::HeightfieldCollider(int32_t heightStickWidth, int32_t heightStickLength, ukk heightfieldData, float minHeight,
	float maxHeight, bool flipQuadEdges, const Transform &localTransform) /*:
	Collider(localTransform)*/ {
	this->localTransform = localTransform;
	SetHeightfield(heightStickWidth, heightStickLength, heightfieldData, minHeight, maxHeight, flipQuadEdges);
}

HeightfieldCollider::~HeightfieldCollider() {
}

CollisionShape *HeightfieldCollider::GetCollisionShape() const {
	return shape.get();
}

void HeightfieldCollider::SetHeightfield(int32_t heightStickWidth, int32_t heightStickLength, ukk heightfieldData, float minHeight, float maxHeight,
	bool flipQuadEdges) {
	if (!heightfieldData) return;

	shape = std::make_unique<HeightfieldTerrainShape>(heightStickWidth, heightStickLength, heightfieldData, 1.0f, minHeight, maxHeight, 1, PHY_FLOAT, flipQuadEdges);
}

const Node &operator>>(const Node &node, HeightfieldCollider &collider) {
	node["localTransform"].Get(collider.localTransform);
	return node;
}

Node &operator<<(Node &node, const HeightfieldCollider &collider) {
	node["localTransform"].Set(collider.localTransform);
	return node;
}
}
