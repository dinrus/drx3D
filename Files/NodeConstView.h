#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <drx3D/Export.h>

namespace drx3d {
class Node;

enum class NodeType : uint8_t {
	Object, Array, String, Boolean, Integer, Decimal, Null, // Type of node value.
	Unknown, Token, EndOfFile, // Used in tokenizers.
};
using NodeValue = STxt;

using NodeProperty = std::pair<STxt, Node>;
using NodeProperties = std::vector<NodeProperty>;

/**
 * @brief Class that is returned from a {@link Node} when getting constant properties. This represents a key tree from a parent,
 * this allows reads of large trees with broken nodes to not need to generate new content.
 */
class DRX3D_EXPORT NodeConstView {
	friend class Node;
protected:
	using Key = std::variant<STxt, uint32_t>;

	NodeConstView() = default;
	NodeConstView(const Node *parent, Key key, const Node *value);
	NodeConstView(const NodeConstView *parent, Key key);

public:
	bool has_value() const noexcept { return value != nullptr; }
	const Node *get() const { return value; }

	explicit operator bool() const noexcept { return has_value(); }
	operator const Node &() const { return *value; }

	const Node &operator*() const { return *value; }
	const Node *operator->() const { return value; }

	template<typename T>
	T Get() const;
	template<typename T>
	T GetWithFallback(const T &fallback) const;
	template<typename T>
	bool Get(T &dest) const;
	template<typename T, typename K>
	bool GetWithFallback(T &dest, const K &fallback) const;
	template<typename T>
	bool Get(T &&dest) const;
	template<typename T, typename K>
	bool GetWithFallback(T &&dest, const K &fallback) const;
	
	NodeConstView GetPropertyWithBackup(const STxt &key, const STxt &backupKey) const;
	NodeConstView GetPropertyWithValue(const STxt &key, const NodeValue &propertyValue) const;

	NodeConstView operator[](const STxt &key) const;
	NodeConstView operator[](uint32_t index) const;

	NodeProperties GetProperties() const;

	NodeType GetType() const;
	
protected:
	const Node *parent = nullptr;
	const Node *value = nullptr;
	std::vector<Key> keys;
};
}
