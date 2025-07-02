#pragma once

#include <drx3D/Graphics/Subrender.h>
#include <drx3D/Graphics/Graphics.h>
#include <drx3D/Graphics/Descriptors/DescriptorsHandler.h>
#include <drx3D/Graphics/Pipelines/PipelineGraphics.h>

namespace drx3d {
/**
 * @brief Represents a post effect pipeline.
 */
class DRX3D_EXPORT PostFilter : public Subrender {
public:
	/**
	 * Creates a new post filter.
	 * @param pipelineStage The pipelines graphics stage.
	 * @param shaderStages The pipelines shader stages.
	 * @param defines A list of names that will be added as a define.
	 */
	PostFilter(const Pipeline::Stage &pipelineStage, const std::vector<std::filesystem::path> &shaderStages, const std::vector<Shader::Define> &defines = {});
	virtual ~PostFilter() = default;

	const DescriptorsHandler &GetDescriptorSet() const { return descriptorSet; }
	const PipelineGraphics &GetPipeline() const { return pipeline; }

	const Descriptor *GetAttachment(const STxt &descriptorName, const Descriptor *descriptor) const;
	const Descriptor *GetAttachment(const STxt &descriptorName, const STxt &rendererAttachment) const;

	void SetAttachment(const STxt &descriptorName, const Descriptor *descriptor);
	bool RemoveAttachment(const STxt &name);

protected:
	/**
	 * Used instead of {@link DescriptorsHandler#Push} in instances where a writeColor is the same as samplerColor in a shader.
	 * By switching between what will be the input and output of each filter previous changes are available to the shader.
	 * @param descriptorName1 The first descriptor in the shader.
	 * @param descriptorName2 The second descriptor in the shader.
	 * @param rendererAttachment1 The name of the renderers attachment that will be first option.
	 * @param rendererAttachment2 The name of the renderers attachment that will be second option.
	 */
	void PushConditional(const STxt &descriptorName1, const STxt &descriptorName2, const STxt &rendererAttachment1, const STxt &rendererAttachment2);

	inline static uint32_t GlobalSwitching = 0;

	DescriptorsHandler descriptorSet;
	PipelineGraphics pipeline;

	std::map<STxt, const Descriptor *> attachments;
};
}
