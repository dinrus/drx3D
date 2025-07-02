#pragma once

#include <drx3D/Maths/Vector2.h>
#include <drx3D/Graphics/Pipelines/Shader.h>

namespace drx3d {
class DRX3D_EXPORT Vertex2d {
public:
	Vertex2d() = default;
	Vertex2d(const Vector2f &position, const Vector2f &uv) :
		position(position),
		uv(uv) {
	}

	static Shader::VertexInput GetVertexInput(uint32_t baseBinding = 0) {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			{baseBinding, sizeof(Vertex2d), VK_VERTEX_INPUT_RATE_VERTEX}
		};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			{0, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2d, position)},
			{1, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex2d, uv)}
		};
		return {bindingDescriptions, attributeDescriptions};
	}

	bool operator==(const Vertex2d &rhs) const {
		return position == rhs.position && uv == rhs.uv;
	}

	bool operator!=(const Vertex2d &rhs) const {
		return !operator==(rhs);
	}

	Vector2f position;
	Vector2f uv;
};
}

namespace std {
template<>
struct hash<drx3d::Vertex2d> {
	size_t operator()(const drx3d::Vertex2d &vertex) const noexcept {
		size_t seed = 0;
		drx3d::Maths::HashCombine(seed, vertex.position);
		drx3d::Maths::HashCombine(seed, vertex.uv);
		return seed;
	}
};
}
