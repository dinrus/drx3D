#include <drx3D/Skyboxes/SkyboxMaterial.h>

#include <drx3D/Scenes/Scenes.h>

namespace drx3d {
SkyboxMaterial::SkyboxMaterial(std::shared_ptr<ImageCube> image, const Color &baseColor) :
	image(std::move(image)),
	baseColor(baseColor),
	blend(1.0f),
	fogLimits(-10000.0f) {
}

void SkyboxMaterial::CreatePipeline(const Shader::VertexInput &vertexInput, bool animated) {
	pipelineMaterial = MaterialPipeline::Create({1, 0}, {
		{"Shaders/Skyboxes/Skybox.vert", "Shaders/Skyboxes/Skybox.frag"},
		{vertexInput}, {}, PipelineGraphics::Mode::MRT, PipelineGraphics::Depth::None,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT
	});
}

void SkyboxMaterial::PushUniforms(UniformHandler &uniformObject, const Transform *transform) {
	if (transform) {
		uniformObject.Push("transform", transform->GetWorldMatrix());
		uniformObject.Push("fogLimits", transform->GetScale().y * fogLimits);
	}
	
	uniformObject.Push("baseColor", baseColor);
	uniformObject.Push("fogColor", fogColor);
	uniformObject.Push("blendFactor", blend);
}

void SkyboxMaterial::PushDescriptors(DescriptorsHandler &descriptorSet) {
	descriptorSet.Push("samplerColor", image);
}

const Node &operator>>(const Node &node, SkyboxMaterial &material) {
	node["image"].Get(material.image);
	node["baseColor"].Get(material.baseColor);
	return node;
}

Node &operator<<(Node &node, const SkyboxMaterial &material) {
	node["image"].Set(material.image);
	node["baseColor"].Set(material.baseColor);
	return node;
}
}
