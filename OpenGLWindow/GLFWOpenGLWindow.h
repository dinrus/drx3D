
#ifndef GLFW_OPENGL_WINDOW_H
#define GLFW_OPENGL_WINDOW_H

#ifdef D3_USE_GLFW

#include <drx3D/Common/Interfaces/CommonWindowInterface.h>

#define b3gDefaultOpenGLWindow GLFWOpenGLWindow

class GLFWOpenGLWindow : public CommonWindowInterface
{
	struct GLFWOpenGLWindowInternalData* m_data;

protected:
public:
	GLFWOpenGLWindow();

	virtual ~GLFWOpenGLWindow();

	virtual void createDefaultWindow(i32 width, i32 height, tukk title);

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

	void keyboardCallbackInternal(i32 key, i32 state);
	void mouseButtonCallbackInternal(i32 button, i32 state);
	void mouseCursorCallbackInternal(double xPos, double yPos);
	void resizeInternal(i32 width, i32 height);
};
#endif  //D3_USE_GLFW
#endif  //GLFW_OPENGL_WINDOW_H
