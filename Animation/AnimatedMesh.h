#pragma once

#include <drx3D/Materials/Material.h>
#include <drx3D/Geometry/Model.h>
#include <drx3D/Scenes/Component.h>
#include <drx3D/Graphics/Buffers/StorageHandler.h>
#include <drx3D/Animation/Geometry/VertexAnimated.h>
#include <drx3D/Animation/Animator.h>

namespace drx3d {

class DRX3D_EXPORT AnimatedMesh : public Component::Registry<AnimatedMesh> {
	inline static const bool Registered = Register("animatedMesh");
public:

	explicit AnimatedMesh(std::filesystem::path filename = "", std::unique_ptr<Material> &&material = nullptr);

	void Start() override;
	void Update() override;

	bool CmdRender(const CommandBuffer &commandBuffer, UniformHandler &uniformScene, const Pipeline::Stage &pipelineStage);

	static Shader::VertexInput GetVertexInput(uint32_t binding = 0) { return VertexAnimated::GetVertexInput(binding); }

	const std::shared_ptr<Model> &GetModel() const { return model; }
	void SetModel(const std::shared_ptr<Model> &model) { this->model = model; }

	const std::unique_ptr<Material> &GetMaterial() const { return material; }
	void SetMaterial(std::unique_ptr<Material> &&material);

	StorageHandler &GetStorageAnimation() { return storageAnimation; }

	friend const Node &operator>>(const Node &node, AnimatedMesh &animatedMesh);
	friend Node &operator<<(Node &node, const AnimatedMesh &animatedMesh);

	constexpr static uint32_t MaxJoints = 50;
	constexpr static uint32_t MaxWeights = 3;

private:
	std::shared_ptr<Model> model;
	std::unique_ptr<Material> material;
	
	std::filesystem::path filename;
	Animator animator;
	Joint headJoint;
	
	std::unique_ptr<Animation> animation;

	DescriptorsHandler descriptorSet;
	UniformHandler uniformObject;
	StorageHandler storageAnimation;
};
}
