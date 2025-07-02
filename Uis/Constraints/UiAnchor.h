#pragma once

#include <drx3D/Export.h>

namespace drx3d {
class DRX3D_EXPORT UiAnchor {
public:
	explicit constexpr UiAnchor(float value) : value(value) {}

	constexpr float Get() const { return value; }

	bool operator==(const UiAnchor &rhs) const {
		return value == rhs.value;
	}

	bool operator!=(const UiAnchor &rhs) const {
		return !operator==(rhs);
	}

	static const UiAnchor Zero;
	static const UiAnchor Left;
	static const UiAnchor Top;
	static const UiAnchor Centre;
	static const UiAnchor Right;
	static const UiAnchor Bottom;

private:
	float value;
};
}
