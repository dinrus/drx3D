#pragma once

#include "Node.h"

namespace drx3d {
template<typename T>
void NodeView::Set(const T &value) {
	get()->Set<T>(value);
}

template<typename T>
void NodeView::Set(T &&value) {
	get()->Set<std::remove_reference_t<T>>(std::move(value));
}

template<typename T>
Node &NodeView::operator=(const T &rhs) {
	return *get() = rhs;
}

template<typename T>
Node &NodeView::operator=(T &&rhs) {
	return *get() = std::move(rhs);
}
}
