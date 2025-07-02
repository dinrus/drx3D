#pragma once

#include <drx3D/Maths/Transform.h>
#include <drx3D/Gizmos/GizmoType.h>

namespace drx3d {
/**
 * @brief A instance of a gizmo type.
 */
class DRX3D_EXPORT Gizmo {
	friend class GizmoType;
public:
	/**
	 * Creates a new gizmo object.
	 * @param gizmoType The gizmo template to build from.
	 * @param transform The gizmos initial transform.
	 * @param color The color for this gizmo, without a value it will be set to the types default.
	 */
	Gizmo(const std::shared_ptr<GizmoType> &gizmoType, const Transform &transform, const std::optional<Color> &color = {});

	bool operator==(const Gizmo &rhs) const;
	bool operator!=(const Gizmo &rhs) const;

	std::shared_ptr<GizmoType> GetGizmoType() const { return gizmoType; }

	const Transform &GetTransform() const { return transform; }
	Transform &GetTransform() { return transform; }
	void SetTransform(const Transform &transform) { this->transform = transform; }

	const Color &GetColor() const { return color; }
	void SetColor(const Color &color) { this->color = color; }

private:
	std::shared_ptr<GizmoType> gizmoType;
	Transform transform;
	Color color;
};
}
