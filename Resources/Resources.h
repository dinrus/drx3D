#pragma once

#include <unordered_map>

#include <drx3D/Engine/Engine.h>
#include <drx3D/Common/ThreadPool.h>
#include <drx3D/Files/Files.h>
#include <drx3D/Files/Node.h>
#include <drx3D/Resources/Resource.h>

namespace drx3d {
/**
 * @brief Module used for managing resources. Resources are held alive as long as they are in use,
 * a existing resource is queried by node value.
 */
class DRX3D_EXPORT Resources : public Module::Registry<Resources> {
	inline static const bool Registered = Register(Stage::Post, Requires<Files>());
public:
	Resources();

	void Update() override;

	std::shared_ptr<Resource> Find(const std::type_index &typeIndex, const Node &node) const;

	template<typename T>
	std::shared_ptr<T> Find(const Node &node) const {
		if (resources.find(typeid(T)) == resources.end())
			return nullptr;
		
		for (const auto &[key, resource] : resources.at(typeid(T))) {
			if (key == node)
				return std::dynamic_pointer_cast<T>(resource);
		}
		
		return nullptr;
	}
	
	void Add(const Node &node, const std::shared_ptr<Resource> &resource);
	void Remove(const std::shared_ptr<Resource> &resource);

	/**
	 * Gets the resource loader thread pool.
	 * @return The resource loader thread pool.
	 */
	ThreadPool &GetThreadPool() { return threadPool; }

private:
	std::unordered_map<std::type_index, std::map<Node, std::shared_ptr<Resource>>> resources;
	ElapsedTime elapsedPurge;

	ThreadPool threadPool;
};
}
