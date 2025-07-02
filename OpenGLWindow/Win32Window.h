#ifndef _WIN32_WINDOW_H
#define _WIN32_WINDOW_H

struct InternalData2;

#include <drx3D/Common/Interfaces/CommonWindowInterface.h>

class Win32Window : public CommonWindowInterface
{
protected:
	struct InternalData2* m_data;

	void pumpMessage();

public:
	Win32Window();

	virtual ~Win32Window();

	virtual void createWindow(const b3gWindowConstructionInfo& ci);

	virtual void switchFullScreen(bool fullscreen, i32 width = 0, i32 height = 0, i32 colorBitsPerPixel = 0);

	virtual void closeWindow();

	virtual void runMainLoop();

	virtual void startRendering();

	virtual void renderAllObjects();

	virtual void endRendering();

	virtual float getTimeInSeconds();

	virtual void setDebugMessage(i32 x, i32 y, tukk message);

	virtual bool requestedExit() const;

	virtual void setRequestExit();

	virtual void getMouseCoordinates(i32& x, i32& y);

	virtual void setMouseMoveCallback(b3MouseMoveCallback mouseCallback);
	virtual void setMouseButtonCallback(b3MouseButtonCallback mouseCallback);
	virtual void setResizeCallback(b3ResizeCallback resizeCallback);
	virtual void setWheelCallback(b3WheelCallback wheelCallback);
	virtual void setKeyboardCallback(b3KeyboardCallback keyboardCallback);

	virtual b3MouseMoveCallback getMouseMoveCallback();
	virtual b3MouseButtonCallback getMouseButtonCallback();
	virtual b3ResizeCallback getResizeCallback();
	virtual b3WheelCallback getWheelCallback();
	virtual b3KeyboardCallback getKeyboardCallback();

	virtual void setRenderCallback(b3RenderCallback renderCallback);

	virtual void setWindowTitle(tukk title);

	virtual bool isModifierKeyPressed(i32 key);
};

#endif  //_WIN32_WINDOW_H