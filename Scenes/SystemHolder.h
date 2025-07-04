#pragma once

#include <memory>

#include <drx3D/Engine/Log.h>
#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Scenes/System.h>

namespace drx3d {
class DRX3D_EXPORT SystemHolder : NonCopyable {
public:
	/**
	 * Checks whether a System exists or not.
	 * @tparam T The System type.
	 * @return If the System exists.
	 */
	template<typename T,
		typename = std::enable_if_t<std::is_convertible_v<T *, System *>>>
	bool Has() const {
		const auto it = systems.find(TypeInfo<System>::GetTypeId<T>());

		return it != systems.end() && it->second;
	}

	/**
	 * Gets a System.
	 * @tparam T The System type.
	 * @return The System.
	 */
	template<typename T,
		typename = std::enable_if_t<std::is_convertible_v<T *, System *>>>
	T *Get() const {
		auto it = systems.find(TypeInfo<System>::GetTypeId<T>());

		if (it == systems.end() || !it->second) {
			//throw drx::Exc("Scene does not have requested System");
			return nullptr;
		}

		return static_cast<T *>(it->second.get());
	}

	/**
	 * Adds a System.
	 * @tparam T The System type.
	 * @param system The System.
	 */
	template<typename T,
		typename = std::enable_if_t<std::is_convertible_v<T *, System *>>>
	void Add(std::unique_ptr<T> &&system) {
		// Remove previous System, if it exists.
		Remove<T>();

		const auto typeId = TypeInfo<System>::GetTypeId<T>();

		// Then, add the System
		systems[typeId] = std::move(system);
	}

	/**
	 * Removes a System.
	 * @tparam T The System type.
	 */
	template<typename T,
		typename = std::enable_if_t<std::is_convertible_v<T *, System *>>>
	void Remove() {
		const auto typeId = TypeInfo<System>::GetTypeId<T>();

		// Then, remove the System.
		systems.erase(typeId);
	}

	/**
	 * Removes all Systems.
	 */
	void Clear();

	/**
	 * Iterates through all valid Systems.
	 * @tparam Func The function type.
	 * @param func The function to pass each System into, System object and System ID.
	 */
	template<typename Func>
	void ForEach(Func &&func) {
		for (auto &[typeId, system] : systems) {
			try {
				func(typeId, system.get());
			} catch (const std::exception &e) {
				Log::Error(e.what(), '\n');
			}
		}
	}
	
private:
	/// List of all Systems.
	std::unordered_map<TypeId, std::unique_ptr<System>> systems;
};
}
