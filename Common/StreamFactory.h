#pragma once

#include <functional>
#include <iomanip>

#include <drx3D/Engine/Log.h>
#include <drx3D/Files/Node.h>
#include "TypeInfo.h"

namespace drx3d {
template<typename Base, class... Args>
class StreamFactory {
public:
	using TCreateReturn = std::unique_ptr<Base>;

	using TCreateMethod = std::function<TCreateReturn(Args...)>;
	using TRegistryMap = std::unordered_map<STxt, TCreateMethod>;

	virtual ~StreamFactory() = default;

	static TCreateReturn Create(const STxt &name, Args &&... args) {
		auto it = DoRegister().find(name);
		if (it == DoRegister().end()) {
			Log::Error("Failed to create ", std::quoted(name), " from factory\n");
			return nullptr;
		}
		return it->second(std::forward<Args>(args)...);
	}

	static TCreateReturn Create(const Node &node) {
		auto name = node["type"].Get<STxt>();
		auto it = DoRegister().find(name);
		if (it == DoRegister().end()) {
			Log::Error("Failed to create ", std::quoted(name), " from factory\n");
			return nullptr;
		}
		auto ret = it->second();
		node.Get(ret);
		return ret;
	}

	static TRegistryMap &DoRegister() {
		static TRegistryMap impl;
		return impl;
	}

	template<typename T>
	class Registry : public Base {
	public:
		TypeId GetTypeId() const override { return TypeInfo<Base>::template GetTypeId<T>(); }
		STxt GetTypeName() const override { return name; }

	protected:
		static bool Register(const STxt &name) {
			Registry::name = name;
			StreamFactory::DoRegister()[name] = [](Args... args) -> TCreateReturn {
				return std::make_unique<T>(std::forward<Args>(args)...);
			};
			return true;
		}

		const Node &Load(const Node &node) override {
			return node >> *dynamic_cast<T *>(this);
		}
		
		Node &Write(Node &node) const override {
			node["type"].Set(name);
			return node << *dynamic_cast<const T *>(this);
		}

		inline static STxt name;
	};

	friend const Node &operator>>(const Node &node, std::unique_ptr<Base> &object) {
		if (node["type"].has_value())
			object = Create(node["type"].Get<STxt>());
		node >> *object;
		return node;
	}

	virtual TypeId GetTypeId() const { return -1; }
	virtual STxt GetTypeName() const { return ""; }

	friend const Node &operator>>(const Node &node, Base &base) {
		return base.Load(node);
	}
	
	friend Node &operator<<(Node &node, const Base &base) {
		return base.Write(node);
	}
	
protected:
	virtual const Node &Load(const Node &node) { return node; }
	virtual Node &Write(Node &node) const { return node; }
};
}
