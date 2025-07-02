#pragma once

#include <drx3D/Scenes/Component.h>
#include <drx3D/Graphics/Descriptors/DescriptorsHandler.h>
#include <drx3D/Graphics/Buffers/PushHandler.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>

namespace drx3d {
/**
 * @brief Component that is used to render a entity as a shadow.
 */
class DRX3D_EXPORT ShadowRender : public Component::Registry<ShadowRender> {
	inline static const bool Registered = Register("shadowRender");
public:
	ShadowRender();

	void Start() override;
	void Update() override;

	bool CmdRender(const CommandBuffer &commandBuffer, const PipelineGraphics &pipeline);

	friend const Node &operator>>(const Node &node, ShadowRender &shadowRender);
	friend Node &operator<<(Node &node, const ShadowRender &shadowRender);

private:
	DescriptorsHandler descriptorSet;
	PushHandler pushObject;
};
}
