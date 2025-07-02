#include <drx3D/Geometry/Gltf/GltfModel.h>

#include <X/tiny/tiny_gltf.h>

#include <drx3D/Files/Files.h>
#include <drx3D/Resources/Resources.h>
#include <drx3D/Geometry/Vertex3d.h>

namespace drx3d {
std::shared_ptr<GltfModel> GltfModel::Create(const Node &node) {
	if (auto resource = Resources::Get()->Find<GltfModel>(node))
		return resource;

	auto result = std::make_shared<GltfModel>("");
	Resources::Get()->Add(node, std::dynamic_pointer_cast<Resource>(result));
	node >> *result;
	result->Load();
	return result;
}

std::shared_ptr<GltfModel> GltfModel::Create(const std::filesystem::path &filename) {
	GltfModel temp(filename, false);
	Node node;
	node << temp;
	return Create(node);
}

GltfModel::GltfModel(std::filesystem::path filename, bool load) :
	filename(std::move(filename)) {
	if (load) {
		Load();
	}
}

const Node &operator>>(const Node &node, GltfModel &model) {
	node["filename"].Get(model.filename);
	return node;
}

Node &operator<<(Node &node, const GltfModel &model) {
	node["filename"].Set(model.filename);
	return node;
}

void GltfModel::Load() {
	if (filename.empty()) {
		return;
	}

#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto folder = filename.parent_path();
	auto fileLoaded = Files::Read(filename);

	if (!fileLoaded) {
		Log::Error("Не удалось загрузить модель: ", filename, '\n');
		return;
	}

	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF gltfContext;
	STxt warn, err;

	if (filename.extension() == ".glb") {
		if (!gltfContext.LoadBinaryFromMemory(&gltfModel, &err, &warn, reinterpret_cast<uint8_t *>(fileLoaded->data()), static_cast<uint32_t>(fileLoaded->size()))) {
			throw drx::Exc(warn + err);
		}
	} else {
		if (!gltfContext.LoadASCIIFromString(&gltfModel, &err, &warn, fileLoaded->c_str(), static_cast<uint32_t>(fileLoaded->size()), folder.string())) {
			throw drx::Exc(warn + err);
		}
	}

	std::vector<Vertex3d> vertices;
	std::vector<uint32_t> indices;
	std::unordered_map<Vertex3d, size_t> uniqueVertices;

	//LoadTextureSamplers(gltfModel);
	//LoadTextures(gltfModel);
	//LoadMaterials(gltfModel);
	auto scale = 1.0f;

	// TODO: Scene handling with no default scene.
	/*const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

	for (std::size_t i = 0; i < scene.nodes.size(); i++) {
		const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
		LoadNode(nullptr, node, scene.nodes[i], gltfModel, indices, vertices, scale);
	}

	if (gltfModel.animations.size() > 0) {
		LoadAnima(gltfModel);
	}

	LoadSkins(gltfModel);

	for (auto node : linearNodes) {
		// Assign skins.
		if (node->skinIndex > -1) {
			node->skin = skins[node->skinIndex];
		}

		// Initial pose.
		if (node->mesh) {
			node->update();
		}
	}

	auto extensions = gltfModel.extensionsUsed;*/

#ifdef DRX3D_DEBUG
	Log::Out("Модель ", filename, " загружена за ", (Time::Now() - debugStart).AsMilliseconds<float>(), "мс\n");
#endif

	Initialize(vertices, indices);
}
}
