#include <drx3D/Uis/UiPanel.h>

#include <drx3D/Uis/Inputs/UiButtonInput.h>
#include <drx3D/Uis/Drivers/ConstantDriver.h>

namespace drx3d {
constexpr static Vector2i RESIZE_SIZE(16, 16);
constexpr static Vector2i PADDING(16, 16);

UiPanel::UiPanel() {
	//background.SetTransform({UiMargins::All});
	background.SetImage(Image2d::Create("Guis/White.png"));
	UiObject::AddChild(&background);

	//content.SetTransform({UiMargins::None, PADDING, -PADDING});
	UiObject::AddChild(&content);

	//resizeHandle.SetTransform({RESIZE_SIZE, UiAnchor::RightBottom});
	resizeHandle.SetImage(Image2d::Create("Guis/White.png"));
	resizeHandle.SetColorDriver<ConstantDriver>(UiButtonInput::ButtonColor);
	resizeHandle.SetCursorHover(std::make_unique<Cursor>(CursorStandard::ResizeX));
	UiObject::AddChild(&resizeHandle);

	//scrollX.SetTransform({UiMargins::None, {}, {-RESIZE_SIZE.x, 0}});
	scrollX.SetType(ScrollBar::Horizontal);
	UiObject::AddChild(&scrollX);

	//scrollY.SetTransform({UiMargins::None, {}, {0, -RESIZE_SIZE.y}});
	scrollY.SetType(ScrollBar::Vertical);
	UiObject::AddChild(&scrollY);
}

void UiPanel::UpdateObject() {
	resizeHandle.SetEnabled(manipulate & UiManipulate::Resize);
	auto contentSize = (max - min) / GetScreenSize();
	scrollX.SetEnabled(scrollBars & ScrollBar::Horizontal && contentSize.x > 1.05f);
	scrollY.SetEnabled(scrollBars & ScrollBar::Vertical && contentSize.y > 1.05f);

	// TODO: Abstract math into UiTransform.
//	scrollX.GetTransform().SetAnchor0({0, GetScreenSize().y - UiScrollBar::Size});
//	scrollY.GetTransform().SetAnchor0({GetScreenSize().x - UiScrollBar::Size, 0});

	scrollX.SetSize({-1.0f / contentSize.x, 0.0f});
	scrollY.SetSize({0.0f, -1.0f / contentSize.y});

	//content.GetTransform().SetPosition(0.5f - (Vector2f(scrollX.GetProgress(), scrollY.GetProgress()) * contentSize));

	min = Vector2f::Infinity;
	max = -Vector2f::Infinity;
	//SetScissor(&scrollX);
	//SetScissor(&scrollY);
	SetScissor(&content, true);
}

void UiPanel::AddChild(UiObject *child) {
	content.AddChild(child);
}

void UiPanel::SetBackgroundColor(const Color &color) {
	background.SetColorDriver<ConstantDriver>(color);
}

void UiPanel::SetScissor(UiObject *object, bool checkSize) {
	auto position = background.GetScreenPosition();
	auto size = background.GetScreenSize();
	object->SetScissor(Vector4i(position, size));

	if (object->IsEnabled() && checkSize) {
		min = min.d3Min(object->GetScreenPosition());
		max = max.d3Max(object->GetScreenPosition() + object->GetScreenSize());
	}

	for (auto &child : object->GetChildren()) {
		SetScissor(child, checkSize);
	}
}
}
