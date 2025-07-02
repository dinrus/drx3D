#pragma once

#include <drx3D/Maths/Vector2.h>
#include <drx3D/Post/PostFilter.h>

namespace drx3d {
class DRX3D_EXPORT BlurFilter : public PostFilter {
public:
	enum class Type {
		_5 = 5,
		_9 = 9,
		_13 = 13
	};

	explicit BlurFilter(const Pipeline::Stage &pipelineStage, const Vector2f &direction, Type type = Type::_9);

	void Render(const CommandBuffer &commandBuffer) override;

	const Vector2f &GetDirection() const { return direction; }
	void SetDirection(const Vector2f &direction) { this->direction = direction; }

private:
	std::vector<Shader::Define> GetDefines(const Type &type);

	PushHandler pushScene;

	Type type;
	Vector2f direction;
};
}
