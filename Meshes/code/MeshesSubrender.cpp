#include <drx3D/Meshes/MeshesSubrender.h>

#include <drx3D/Animation/AnimatedMesh.h>
#include <drx3D/Scenes/Scenes.h>
#include <drx3D/Meshes/Mesh.h>

namespace drx3d {
MeshesSubrender::MeshesSubrender(const Pipeline::Stage &pipelineStage, Sort sort) :
	Subrender(pipelineStage),
	sort(sort),
	uniformScene(true) {
}

void MeshesSubrender::Render(const CommandBuffer &commandBuffer) {
	auto camera = Scenes::Get()->GetScene()->GetCamera();
	uniformScene.Push("projection", camera->GetProjectionMatrix());
	uniformScene.Push("view", camera->GetViewMatrix());
	uniformScene.Push("cameraPos", camera->GetPosition());

	auto meshes = Scenes::Get()->GetScene()->QueryComponents<Mesh>();
	if (sort == Sort::Front)
		std::sort(meshes.begin(), meshes.end(), std::greater<>());
	else if (sort == Sort::Back)
		std::sort(meshes.begin(), meshes.end(), std::less<>());

	for (const auto &mesh : meshes)
		mesh->CmdRender(commandBuffer, uniformScene, GetStage());

	// TODO: Split animated meshes into it's own subrender.
	auto animatedMeshes = Scenes::Get()->GetScene()->QueryComponents<AnimatedMesh>();
	for (const auto &animatedMesh : animatedMeshes)
		animatedMesh->CmdRender(commandBuffer, uniformScene, GetStage());
}
}
