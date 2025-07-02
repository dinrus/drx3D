#include <drx3D/Gizmos/Gizmo.h>

namespace drx3d {
Gizmo::Gizmo(const std::shared_ptr<GizmoType> &gizmoType, const Transform &transform, const std::optional<Color> &color) :
	gizmoType(gizmoType),
	transform(transform),
	color(color ? *color : gizmoType->GetColor()) {
}

bool Gizmo::operator==(const Gizmo &rhs) const {
	return gizmoType == rhs.gizmoType && transform == rhs.transform && color == rhs.color;
}

bool Gizmo::operator!=(const Gizmo &rhs) const {
	return !operator==(rhs);
}
}
