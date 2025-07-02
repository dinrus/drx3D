#ifndef EGL_OPENGL_WINDOW_H
#define EGL_OPENGL_WINDOW_H

#ifdef DRX3D_USE_EGL

#include <drx3D/Common/Interfaces/CommonWindowInterface.h>

class EGLOpenGLWindow : public CommonWindowInterface
{
	struct EGLInternalData2* m_data;
	bool m_OpenGLInitialized;
	bool m_requestedExit;

public:
	EGLOpenGLWindow();
	virtual ~EGLOpenGLWindow();

	virtual void createDefaultWindow(i32 width, i32 height, tukk title)
	{
		b3gWindowConstructionInfo ci(width, height);
		ci.m_title = title;
		createWindow(ci);
	}

	virtual void createWindow(const b3gWindowConstructionInfo& ci);

	virtual void closeWindow();

	virtual void runMainLoop();
	virtual float getTimeInSeconds();

	virtual bool requestedExit() const;
	virtual void setRequestExit();

	virtual void startRendering();

	virtual void endRendering();

	virtual bool isModifierKeyPressed(i32 key);

	virtual void setMouseMoveCallback(b3MouseMoveCallback mouseCallback);
	virtual b3MouseMoveCallback getMouseMoveCallback();

	virtual void setMouseButtonCallback(b3MouseButtonCallback mouseCallback);
	virtual b3MouseButtonCallback getMouseButtonCallback();

	virtual void setResizeCallback(b3ResizeCallback resizeCallback);
	virtual b3ResizeCallback getResizeCallback();

	virtual void setWheelCallback(b3WheelCallback wheelCallback);
	virtual b3WheelCallback getWheelCallback();

	virtual void setKeyboardCallback(b3KeyboardCallback keyboardCallback);
	virtual b3KeyboardCallback getKeyboardCallback();

	virtual void setRenderCallback(b3RenderCallback renderCallback);

	virtual void setWindowTitle(tukk title);

	virtual float getRetinaScale() const;
	virtual void setAllowRetina(bool allow);

	virtual i32 getWidth() const;
	virtual i32 getHeight() const;

	virtual i32 fileOpenDialog(tuk fileName, i32 maxFileNameLength);
};

#endif  //DRX3D_USE_EGL

#endif  //EGL_OPENGL_WINDOW_H
