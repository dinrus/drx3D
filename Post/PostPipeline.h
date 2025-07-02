#pragma once

#include <drx3D/Graphics/Subrender.h>

namespace drx3d {
/**
 * @brief Represents a system of post effects.
 */
class DRX3D_EXPORT PostPipeline : public Subrender {
public:
	/**
	 * Creates a new post pipeline.
	 * @param pipelineStage The pipelines graphics stage.
	 */
	explicit PostPipeline(const Pipeline::Stage &pipelineStage) :
		Subrender(pipelineStage) {
	}

	virtual ~PostPipeline() = default;
};
}
