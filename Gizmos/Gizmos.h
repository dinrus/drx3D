#pragma once

#include <drx3D/Scenes/System.h>
#include <drx3D/Gizmos/Gizmo.h>

namespace drx3d {
/**
 * @brief Module used for that manages debug gizmos.
 */
class DRX3D_EXPORT Gizmos : public System {
public:
	using GizmosContainer = std::map<std::shared_ptr<GizmoType>, std::vector<std::unique_ptr<Gizmo>>>;

	Gizmos();

	void Update() override;

	Gizmo *AddGizmo(std::unique_ptr<Gizmo> &&gizmo);
	void RemoveGizmo(Gizmo *gizmo);

	/**
	 * Clears all gizmos from the scene.
	 */
	void Clear();

	/**
	 * Gets a list of all gizmos.
	 * @return All gizmos.
	 */
	const GizmosContainer &GetGizmos() const { return gizmos; }

private:
	GizmosContainer gizmos;
};
}
