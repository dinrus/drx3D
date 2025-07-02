#include <drx3D/Scenes/Scenes.h>

namespace drx3d {
Scenes::Scenes() {
}

void Scenes::Update() {
	if (!scene) return;

	if (!scene->started) {
		scene->Start();
		scene->started = true;
	}

	scene->Update();
}
}
