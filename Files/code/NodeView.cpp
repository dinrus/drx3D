#include <drx3D/Files/NodeView.h>

#include <drx3D/Files/Node.h>

namespace drx3d {
NodeView::NodeView(Node *parent, Key key, Node *value) :
	NodeConstView(parent, std::move(key), value) {
}

NodeView::NodeView(NodeView *parent, Key key) :
	NodeConstView(parent, std::move(key)) {
}

Node *NodeView::get() {
	if (!has_value()) {
		// This will build the tree of nodes from the return keys tree.
		for (const auto &key : keys) {
			if (const auto name = std::get_if<STxt>(&key))
				value = &const_cast<Node *>(parent)->AddProperty(*name);
			else if (const auto index = std::get_if<std::uint32_t>(&key))
				value = &const_cast<Node *>(parent)->AddProperty(*index);
			else
				throw drx::Exc("Key for node return is neither a i32 or a string");
			
			// Because the last key will set parent to the value parent usage should be avoided.
			parent = value;
		}

		keys.erase(keys.begin(), keys.end() - 1);
	}

	return const_cast<Node *>(value);
}

NodeView NodeView::GetPropertyWithBackup(const STxt &key, const STxt &backupKey) {
	if (!has_value())
		return {this, key};
	return const_cast<Node *>(value)->GetPropertyWithBackup(key, backupKey);
}

NodeView NodeView::GetPropertyWithValue(const STxt &key, const NodeValue &propertyValue) {
	if (!has_value())
		return {this, key};
	return const_cast<Node *>(value)->GetPropertyWithValue(key, propertyValue);
}

NodeView NodeView::operator[](const STxt &key) {
	if (!has_value())
		return {this, key};
	return const_cast<Node *>(value)->operator[](key);
}

NodeView NodeView::operator[](uint32_t index) {
	if (!has_value())
		return {this, index};
	return const_cast<Node *>(value)->operator[](index);
}

NodeProperties &NodeView::GetProperties() {
	if (!has_value())
		return get()->GetProperties();
	return const_cast<Node *>(value)->GetProperties();
}
}
