#ifndef MAC_OPENGL_WINDOW_H
#define MAC_OPENGL_WINDOW_H

#include <drx3D/Common/Interfaces/CommonWindowInterface.h"

#define b3gDefaultOpenGLWindow MacOpenGLWindow

class MacOpenGLWindow : public CommonWindowInterface
{
	struct MacOpenGLWindowInternalData* m_internalData;

public:
	MacOpenGLWindow();
	virtual ~MacOpenGLWindow();

	void init(i32 width, i32 height, tukk windowTitle);

	void closeWindow();

	void startRendering();

	void endRendering();  //swap buffers

	virtual bool requestedExit() const;

	virtual void setRequestExit();

	void getMouseCoordinates(i32& x, i32& y);

	void runMainLoop();

	virtual bool isModifierKeyPressed(i32 key);

	void setMouseButtonCallback(b3MouseButtonCallback mouseCallback);

	void setMouseMoveCallback(b3MouseMoveCallback mouseCallback);

	void setResizeCallback(b3ResizeCallback resizeCallback);

	void setKeyboardCallback(b3KeyboardCallback keyboardCallback);

	virtual b3MouseMoveCallback getMouseMoveCallback();

	virtual b3MouseButtonCallback getMouseButtonCallback();

	virtual b3ResizeCallback getResizeCallback();

	virtual b3WheelCallback getWheelCallback();

	b3KeyboardCallback getKeyboardCallback();

	void setWheelCallback(b3WheelCallback wheelCallback);

	float getRetinaScale() const;

	virtual void setAllowRetina(bool allow);

	virtual void createWindow(const b3gWindowConstructionInfo& ci);

	virtual float getTimeInSeconds();

	virtual i32 getWidth() const;
	virtual i32 getHeight() const;

	virtual void setRenderCallback(b3RenderCallback renderCallback);

	virtual void setWindowTitle(tukk title);

	i32 fileOpenDialog(tuk filename, i32 maxNameLength);
};

#endif
