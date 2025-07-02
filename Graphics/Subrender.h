#pragma once

#include <drx3D/Graphics/Commands/CommandBuffer.h>
#include <drx3D/Graphics/Pipelines/Pipeline.h>
#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Common/TypeInfo.h>

namespace drx3d {
/**
 * @brief Represents a render pipeline that is used to render a type of pipeline.
 */
class DRX3D_EXPORT Subrender : NonCopyable {
public:
	/**
	 * Creates a new render pipeline.
	 * @param stage The stage this renderer will be used in.
	 */
	explicit Subrender(Pipeline::Stage stage) :
		stage(std::move(stage)) {
	}

	virtual ~Subrender() = default;

	/**
	 * Runs the render pipeline in the current renderpass.
	 * @param commandBuffer The command buffer to record render command into.
	 */
	virtual void Render(const CommandBuffer &commandBuffer) = 0;

	const Pipeline::Stage &GetStage() const { return stage; }

	bool IsEnabled() const { return enabled; }
	void SetEnabled(bool enable) { this->enabled = enable; }

private:
	bool enabled = true;
	Pipeline::Stage stage;
};

template class DRX3D_EXPORT TypeInfo<Subrender>;
}
