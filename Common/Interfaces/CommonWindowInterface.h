#ifndef B3G_WINDOW_INTERFACE_H
#define B3G_WINDOW_INTERFACE_H

#include <drxtypes.h>
#include "CommonCallbacks.h"

struct b3gWindowConstructionInfo
{
	i32 m_width;
	i32 m_height;
	bool m_fullscreen;
	i32 m_colorBitsPerPixel;
	uk m_windowHandle;
	tukk m_title;
	i32 m_openglVersion;
	i32 m_renderDevice;

	b3gWindowConstructionInfo(i32 width = 1024, i32 height = 768)
		: m_width(width),
		  m_height(height),
		  m_fullscreen(false),
		  m_colorBitsPerPixel(32),
		  m_windowHandle(0),
		  m_title("title"),
		  m_openglVersion(3),
		  m_renderDevice(-1)
	{
	}
};

class CommonWindowInterface
{
public:
	virtual ~CommonWindowInterface()
	{
	}

	virtual void createDefaultWindow(i32 width, i32 height, tukk title)
	{
		b3gWindowConstructionInfo ci(width, height);
		ci.m_title = title;
		createWindow(ci);
	}

	virtual void createWindow(const b3gWindowConstructionInfo& ci) = 0;

	virtual void closeWindow() = 0;

	virtual void runMainLoop() = 0;
	virtual float getTimeInSeconds() = 0;

	virtual bool requestedExit() const = 0;
	virtual void setRequestExit() = 0;

	virtual void startRendering() = 0;

	virtual void endRendering() = 0;

	virtual bool isModifierKeyPressed(i32 key) = 0;

	virtual void setMouseMoveCallback(b3MouseMoveCallback mouseCallback) = 0;
	virtual b3MouseMoveCallback getMouseMoveCallback() = 0;

	virtual void setMouseButtonCallback(b3MouseButtonCallback mouseCallback) = 0;
	virtual b3MouseButtonCallback getMouseButtonCallback() = 0;

	virtual void setResizeCallback(b3ResizeCallback resizeCallback) = 0;
	virtual b3ResizeCallback getResizeCallback() = 0;

	virtual void setWheelCallback(b3WheelCallback wheelCallback) = 0;
	virtual b3WheelCallback getWheelCallback() = 0;

	virtual void setKeyboardCallback(b3KeyboardCallback keyboardCallback) = 0;
	virtual b3KeyboardCallback getKeyboardCallback() = 0;

	virtual void setRenderCallback(b3RenderCallback renderCallback) = 0;

	virtual void setWindowTitle(tukk title) = 0;

	virtual float getRetinaScale() const = 0;
	virtual void setAllowRetina(bool allow) = 0;

	virtual i32 getWidth() const = 0;
	virtual i32 getHeight() const = 0;

	virtual i32 fileOpenDialog(tuk fileName, i32 maxFileNameLength) = 0;
};

#endif  //B3G_WINDOW_INTERFACE_H
