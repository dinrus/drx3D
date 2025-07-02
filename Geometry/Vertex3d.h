#pragma once

#include <drx3D/Maths/Vector2.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Graphics/Pipelines/Shader.h>

namespace drx3d {
class DRX3D_EXPORT Vertex3d {
public:
	Vertex3d() = default;
	Vertex3d(const Vector3f &position, const Vector2f &uv, const Vector3f &normal) :
		position(position),
		uv(uv),
		normal(normal) {
	}

	static Shader::VertexInput GetVertexInput(uint32_t baseBinding = 0) {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			{baseBinding, sizeof(Vertex3d), VK_VERTEX_INPUT_RATE_VERTEX}
		};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			{0, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3d, position)},
			{1, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3d, uv)},
			{2, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3d, normal)}
		};
		return {bindingDescriptions, attributeDescriptions};
	}

	bool operator==(const Vertex3d &rhs) const {
		return position == rhs.position && uv == rhs.uv && normal == rhs.normal;
	}

	bool operator!=(const Vertex3d &rhs) const {
		return !operator==(rhs);
	}

	Vector3f position;
	Vector2f uv;
	Vector3f normal;
};
}

namespace std {
template<>
struct hash<drx3d::Vertex3d> {
	size_t operator()(const drx3d::Vertex3d &vertex) const noexcept {
		size_t seed = 0;
		drx3d::Maths::HashCombine(seed, vertex.position);
		drx3d::Maths::HashCombine(seed, vertex.uv);
		drx3d::Maths::HashCombine(seed, vertex.normal);
		return seed;
	}
};
}
