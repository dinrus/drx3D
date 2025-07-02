#pragma once

#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Common/TypeInfo.h>

namespace drx3d {
class DRX3D_EXPORT System : NonCopyable {
public:
	virtual ~System() = default;

	virtual void Update() = 0;

	bool IsEnabled() const { return enabled; }
	void SetEnabled(bool enable) { this->enabled = enable; }

private:
	bool enabled = true;
};

template class DRX3D_EXPORT TypeInfo<System>;
}
