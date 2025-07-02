#pragma once

#include <bitset>
#include <unordered_map>
#include <memory>
#include <functional>

#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Common/TypeInfo.h>

namespace drx3d {
template<typename Base>
class ModuleFactory {
public:
	class TCreateValue {
	public:
		std::function<std::unique_ptr<Base>()> create;
		typename Base::Stage stage;
		std::vector<TypeId> requires_;
	};
	using TRegistryMap = std::unordered_map<TypeId, TCreateValue>;

	virtual ~ModuleFactory() = default;

	static TRegistryMap &DoRegister() {
		static TRegistryMap impl;
		return impl;
	}

	template<typename ... Args>
	class Requires {
	public:
		std::vector<TypeId> Get() const {
			std::vector<TypeId> requires_;
			(requires_.emplace_back(TypeInfo<Base>::template GetTypeId<Args>()), ...);
			return requires_;
		}
	};

	template<typename T>
	class Registry : public Base {
	public:
		/**
		 * Virtual deconstructor called from the engine to clear the instance pointer.
		 */
		virtual ~Registry() {
			if (static_cast<T *>(this) == moduleInstance)
				moduleInstance = nullptr;
		}

		/**
		 * Gets the engines instance.
		 * @return The current module instance.
		 */
		static T *Get() { return moduleInstance; }

	protected:
		/**
		 * Creates a new module singleton instance and registers into the module registry map.
		 * @tparam Args Modules that will be initialized before this module.
		 * @return A dummy value in static initialization.
		 */
		template<typename ... Args>
		static bool Register(typename Base::Stage stage, Requires<Args...> &&requires_ = {}) {
			ModuleFactory::DoRegister()[TypeInfo<Base>::template GetTypeId<T>()] = {[]() {
				moduleInstance = new T();
				// The registrar does not own the instance, the engine does, we just hold a raw pointer for convenience.
				return std::unique_ptr<Base>(moduleInstance);
			}, stage, requires_.Get()};
			return true;
		}
		
		inline static T *moduleInstance = nullptr;
	};
};

/**
 * @brief A interface used for defining engine modules.
 */
class DRX3D_EXPORT Module : public ModuleFactory<Module>, NonCopyable {
public:
	/**
	 * @brief Represents the stage where the module will be updated in the engine.
	 */
	enum class Stage : uint8_t {
		Never, Always, Pre, Normal, Post, Render
	};

	using StageIndex = std::pair<Stage, TypeId>;

	virtual ~Module() = default;

	/**
	 * The update function for the module.
	 */
	virtual void Update() = 0;
};

template class DRX3D_EXPORT TypeInfo<Module>;

class DRX3D_EXPORT ModuleFilter {
public:
	ModuleFilter() {
		// Include all modules by default.
		include.set();
	}
	
	template<typename T>
	bool Check() const noexcept {
		return include.test(TypeInfo<Module>::GetTypeId<T>());
	}

	bool Check(TypeId typeId) const noexcept {
		return include.test(typeId);
	}

	template<typename T>
	ModuleFilter &Exclude() noexcept {
		include.reset(TypeInfo<Module>::GetTypeId<T>());
		return *this;
	}

	template<typename T>
	ModuleFilter &Include() noexcept {
		include.set(TypeInfo<Module>::GetTypeId<T>());
		return *this;
	}

	ModuleFilter &ExcludeAll() noexcept {
		include.reset();
		return *this;
	}

	ModuleFilter &IncludeAll() noexcept {
		include.set();
		return *this;
	}

private:
	std::bitset<64> include;
};
}
