#pragma once

#include <functional>
#include <iomanip>
#include <unordered_map>

#include <drx3D/Engine/Log.h>

namespace drx3d {
template<typename Base, class... Args>
class Factory {
public:
	using TCreateReturn = std::unique_ptr<Base>;

	using TCreateMethod = std::function<TCreateReturn(Args...)>;
	using TRegistryMap = std::unordered_map<STxt, TCreateMethod>;

	virtual ~Factory() = default;

	static TCreateReturn Create(const STxt &name, Args &&... args) {
		auto it = DoRegister().find(name);
		if (it == DoRegister().end()) {
			Log::Error("не удалось создать ", std::quoted(name), " из factory\n");
			return nullptr;
		}
		return it->second(std::forward<Args>(args)...);
	}

	static TRegistryMap &DoRegister() {
		static TRegistryMap impl;
		return impl;
	}

	template<typename T>
	class Registrator : public Base {
	protected:
		static bool Register(const STxt &name) {
			Factory::DoRegister()[name] = [](Args... args) -> TCreateReturn {
				return std::make_unique<T>(std::forward<Args>(args)...);
			};
			return true;
		}
	};
};
}
