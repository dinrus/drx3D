#include <drx3D/Uis/Constraints/UiConstraints.h>

#include <drx3D/Uis/Constraints/PixelConstraint.h>
#include <drx3D/Uis/Constraints/RelativeConstraint.h>

namespace drx3d {
UiConstraints::UiConstraints() {
	SetX<PixelConstraint>(0);
	SetY<PixelConstraint>(0);
	SetWidth<RelativeConstraint>(1.0f);
	SetHeight<RelativeConstraint>(1.0f);
}

bool UiConstraints::Update(const UiConstraints *parent) {
	bool dirty = false;
	dirty |= x->Update(this, parent);
	dirty |= y->Update(this, parent);
	dirty |= width->Update(this, parent);
	dirty |= height->Update(this, parent);
	return dirty;
}
}