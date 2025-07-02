#include <drx3D/Scenes/Scene.h>

namespace drx3d {
Scene::Scene(std::unique_ptr<Camera> &&camera) :
	camera(std::move(camera)) {
}

void Scene::Update() {
	systems.ForEach([](auto typeId, auto system) {
		if (system->IsEnabled())
			system->Update();
	});

	entities.Update();
	camera->Update();
}

void Scene::ClearSystems() {
	systems.Clear();
}

Entity *Scene::GetEntity(const STxt &name) const {
	return entities.GetEntity(name);
}

Entity *Scene::CreateEntity() {
	return entities.CreateEntity();
}

Entity *Scene::CreatePrefabEntity(const STxt &filename) {
	return entities.CreatePrefabEntity(filename);
}

std::vector<Entity *> Scene::QueryAllEntities() {
	return entities.QueryAll();
}

void Scene::ClearEntities() {
	entities.Clear();
}
}
