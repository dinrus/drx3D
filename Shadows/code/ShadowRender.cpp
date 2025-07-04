#include <drx3D/Shadows/ShadowRender.h>

#include <drx3D/Maths/Transform.h>
#include <drx3D/Meshes/Mesh.h>
#include <drx3D/Scenes/Entity.h>
#include <drx3D/Scenes/Scenes.h>
#include <drx3D/Shadows/Shadows.h>

namespace drx3d {
ShadowRender::ShadowRender() {
}

void ShadowRender::Start() {
}

void ShadowRender::Update() {
}

bool ShadowRender::CmdRender(const CommandBuffer &commandBuffer, const PipelineGraphics &pipeline) {
	if (!Scenes::Get()->GetScene()->GetSystem<Shadows>())
		return false;
	
	auto transform = GetEntity()->GetComponent<Transform>();

	if (!transform)
		return false;

	// Update push constants.
	pushObject.Push("mvp", Scenes::Get()->GetScene()->GetSystem<Shadows>()->GetShadowBox().GetProjectionViewMatrix() * transform->GetWorldMatrix());

	// Gets required components.
	auto mesh = GetEntity()->GetComponent<Mesh>();

	if (!mesh || !mesh->GetModel())
		return false;

	// Updates descriptors.
	descriptorSet.Push("PushObject", pushObject);

	if (!descriptorSet.Update(pipeline))
		return false;

	// Draws the object.
	descriptorSet.BindDescriptor(commandBuffer, pipeline);
	pushObject.BindPush(commandBuffer, pipeline);
	return mesh->GetModel()->CmdRender(commandBuffer);
}

const Node &operator>>(const Node &node, ShadowRender &shadowRender) {
	return node;
}

Node &operator<<(Node &node, const ShadowRender &shadowRender) {
	return node;
}
}
