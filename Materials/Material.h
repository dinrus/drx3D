#pragma once

#include <drx3D/Common/StreamFactory.h>
#include <drx3D/Graphics/Descriptors/DescriptorsHandler.h>
#include <drx3D/Graphics/Buffers/UniformHandler.h>
#include <drx3D/Maths/Transform.h>
#include <drx3D/Materials/MaterialPipeline.h>

namespace drx3d {
/**
 * @brief Component that represents a material shader that is used to render a model.
 * The implementation of this object must initialize {@link Material#pipelineMaterial} in {@link Material#CreatePipeline()}.
 */
class DRX3D_EXPORT Material : public StreamFactory<Material> {
public:
	virtual ~Material() = default;

	// TODO: Remove method
	virtual void CreatePipeline(const Shader::VertexInput &vertexInput, bool animated) = 0;

	/**
	 * Used to update the main uniform handler used in a material.
	 * A material can defined it's own uniforms and push them via {@link Material#PushDescriptors()}.
	 * @param uniformObject The uniform handler to update.
	 */
	virtual void PushUniforms(UniformHandler &uniformObject, const Transform *) = 0;

	/**
	 * Used to update a descriptor set containing descriptors used in this materials shader.
	 * @param descriptorSet The descriptor handler to update.
	 */
	virtual void PushDescriptors(DescriptorsHandler &descriptorSet) = 0;

	/**
	 * Gets the material pipeline defined in this material.
	 * @return The material pipeline.
	 */
	const std::shared_ptr<MaterialPipeline> &GetPipelineMaterial() const { return pipelineMaterial; }

protected:
	std::shared_ptr<MaterialPipeline> pipelineMaterial;
};
}
