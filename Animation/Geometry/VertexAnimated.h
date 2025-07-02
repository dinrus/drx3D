#pragma once

#include <drx3D/Maths/Vector2.h>
#include <drx3D/Maths/Vector3.h>
#include <drx3D/Graphics/Pipelines/Shader.h>

namespace drx3d {
class DRX3D_EXPORT VertexAnimated {
public:
	VertexAnimated() = default;

	VertexAnimated(const Vector3f &position, const Vector2f &uv, const Vector3f &normal, const Vector3ui &jointId, const Vector3f &vertexWeight) :
		position(position),
		uv(uv),
		normal(normal),
		jointId(jointId),
		vertexWeight(vertexWeight) {
	}

	static Shader::VertexInput GetVertexInput(uint32_t baseBinding = 0) {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions = {
			{baseBinding, sizeof(VertexAnimated), VK_VERTEX_INPUT_RATE_VERTEX}
		};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			{0, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, position)},
			{1, baseBinding, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexAnimated, uv)},
			{2, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, normal)},
			{3, baseBinding, VK_FORMAT_R32G32B32_SINT, offsetof(VertexAnimated, jointId)},
			{4, baseBinding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexAnimated, vertexWeight)}
		};
		return {bindingDescriptions, attributeDescriptions};
	}

	bool operator==(const VertexAnimated &rhs) const {
		return position == rhs.position && uv == rhs.uv && normal == rhs.normal && jointId == rhs.jointId && vertexWeight == rhs.vertexWeight;
	}

	bool operator!=(const VertexAnimated &rhs) const {
		return !operator==(rhs);
	}

	friend const Node &operator>>(const Node &node, VertexAnimated &vertexAnimated) {
		node["position"].Get(vertexAnimated.position);
		node["uv"].Get(vertexAnimated.uv);
		node["normal"].Get(vertexAnimated.normal);
		node["jointId"].Get(vertexAnimated.jointId);
		node["vertexWeight"].Get(vertexAnimated.vertexWeight);
		return node;
	}

	friend Node &operator<<(Node &node, const VertexAnimated &vertexAnimated) {
		node["position"].Set(vertexAnimated.position);
		node["uv"].Set(vertexAnimated.uv);
		node["normal"].Set(vertexAnimated.normal);
		node["jointId"].Set(vertexAnimated.jointId);
		node["vertexWeight"].Set(vertexAnimated.vertexWeight);
		return node;
	}

	Vector3f position;
	Vector2f uv;
	Vector3f normal;
	Vector3ui jointId;
	Vector3f vertexWeight;
};
}

namespace std {
template<>
struct hash<drx3d::VertexAnimated> {
	size_t operator()(const drx3d::VertexAnimated &vertex) const noexcept {
		size_t seed = 0;
		drx3d::Maths::HashCombine(seed, vertex.position);
		drx3d::Maths::HashCombine(seed, vertex.uv);
		drx3d::Maths::HashCombine(seed, vertex.normal);
		drx3d::Maths::HashCombine(seed, vertex.jointId);
		drx3d::Maths::HashCombine(seed, vertex.vertexWeight);
		return seed;
	}
};
}
