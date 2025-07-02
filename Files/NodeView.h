#pragma once

#include "NodeConstView.h"

namespace drx3d {
/**
 * @brief Class that extends the usage of {@link NodeConstView} to mutable nodes.
 */
class DRX3D_EXPORT NodeView : public NodeConstView {
	friend class Node;
protected:
	NodeView() = default;
	NodeView(Node *parent, Key key, Node *value);
	NodeView(NodeView *parent, Key key);

public:
	Node *get();

	operator Node &() { return *get(); }

	Node &operator*() { return *get(); }
	Node *operator->() { return get(); }

	template<typename T>
	void Set(const T &value);
	template<typename T>
	void Set(T &&value);
	
	NodeView GetPropertyWithBackup(const STxt &key, const STxt &backupKey);
	NodeView GetPropertyWithValue(const STxt &key, const NodeValue &propertyValue);

	NodeView operator[](const STxt &key);
	NodeView operator[](uint32_t index);

	NodeView operator=(const NodeConstView &) = delete;
	NodeView operator=(const NodeView &) = delete;
	template<typename T>
	Node &operator=(const T &rhs);
	template<typename T>
	Node &operator=(T &&rhs);

	NodeProperties &GetProperties();
};
}
