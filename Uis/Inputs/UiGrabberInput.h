#pragma once

#include <drx3D/Devices/Joysticks.h>
#include <drx3D/Devices/Windows.h>
#include <drx3D/Fonts/Text.h>
#include <drx3D/Guis/Gui.h>
#include <drx3D/Uis/UiObject.h>
#include <drx3D/Uis/Inputs/UiButtonInput.h>

namespace drx3d {
class DRX3D_EXPORT UiGrabberInput : public UiObject {
public:
	UiGrabberInput();

	void UpdateObject() override;

	const STxt &GetTitle() const { return textTitle.GetString(); }
	void SetTitle(const STxt &string) { textTitle.SetString(string); }

protected:
	virtual STxt GetTextString() const = 0;

	void SetUpdating(bool updating);
	void UpdateValue();

	Gui background;
	Text textTitle;
	Text textValue;

	int32_t lastKey = 0;

	bool updating = false;
	bool mouseOver = false;
};

class DRX3D_EXPORT UiGrabberJoystick : public UiGrabberInput {
public:
	UiGrabberJoystick();

	JoystickPort GetPort() const { return port; }
	void SetPort(JoystickPort port) { this->port = port; }

	uint32_t GetValue() const { return value; }
	void SetValue(uint32_t value);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(JoystickPort, uint32_t)> &OnValue() { return onValue; }

protected:
	STxt GetTextString() const override {
		return String::To(value);
	}

private:
	JoystickPort port = JoystickPort::_1; // -1
	uint32_t value = 0;
	rocket::signal<void(JoystickPort, uint32_t)> onValue;
};

class DRX3D_EXPORT UiGrabberKeyboard : public UiGrabberInput {
public:
	UiGrabberKeyboard();

	Key GetValue() const { return value; }
	void SetValue(Key value);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(Key)> &OnValue() { return onValue; }

protected:
	STxt GetTextString() const override {
		return Window::ToString(value);
	}

private:
	Key value = Key::Unknown;
	rocket::signal<void(Key)> onValue;
};

class DRX3D_EXPORT UiGrabberMouse : public UiGrabberInput {
public:
	UiGrabberMouse();

	MouseButton GetValue() const { return value; }
	void SetValue(MouseButton value);

	/**
	 * Called when this value of the input changes.
	 * @return The delegate.
	 */
	rocket::signal<void(MouseButton)> &OnValue() { return onValue; }

protected:
	STxt GetTextString() const override {
		return String::To(static_cast<int32_t>(value));
	}

private:
	MouseButton value = MouseButton::_8;
	rocket::signal<void(MouseButton)> onValue;
};
}
