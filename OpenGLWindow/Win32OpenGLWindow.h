#ifndef _WIN32_OPENGL_RENDER_MANAGER_H
#define _WIN32_OPENGL_RENDER_MANAGER_H

#include "Win32Window.h"

#define b3gDefaultOpenGLWindow Win32OpenGLWindow

class Win32OpenGLWindow : public Win32Window
{
	bool m_OpenGLInitialized;

protected:
	void enableOpenGL();

	void disableOpenGL();

public:
	Win32OpenGLWindow();

	virtual ~Win32OpenGLWindow();

	virtual void createWindow(const b3gWindowConstructionInfo& ci);

	virtual void closeWindow();

	virtual void startRendering();

	virtual void renderAllObjects();

	virtual void endRendering();

	virtual float getRetinaScale() const { return 1.f; }
	virtual void setAllowRetina(bool /*allowRetina*/){};

	virtual i32 getWidth() const;
	virtual i32 getHeight() const;

	virtual i32 fileOpenDialog(tuk fileName, i32 maxFileNameLength);
};

#endif  //_WIN32_OPENGL_RENDER_MANAGER_H
