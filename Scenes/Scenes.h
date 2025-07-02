#pragma once

#include <drx3D/Engine/Engine.h>
#include <drx3D/Graphics/Graphics.h>
#include <drx3D/Scenes/Scene.h>

namespace drx3d {
/**
 * @brief Module used for managing game scenes.
 */
class DRX3D_EXPORT Scenes : public Module::Registry<Scenes> {
	// TODO: Scenes should not require Graphics, this is because of Material and Mesh components.
	inline static const bool Registered = Register(Stage::Normal, Requires<Graphics>());
public:
	Scenes();

	void Update() override;

	/**
	 * Gets the current scene.
	 * @return The current scene.
	 */
	Scene *GetScene() const { return scene.get(); }

	/**
	 * Sets the current scene to a new scene.
	 * @param scene The new scene.
	 */
	void SetScene(std::unique_ptr<Scene> &&scene) { this->scene = std::move(scene); }
	
private:
	std::unique_ptr<Scene> scene;
};
}
