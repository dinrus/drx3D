
#ifndef WIN32_INTERNAL_WINDOW_DATA_H
#define WIN32_INTERNAL_WINDOW_DATA_H

#include <windows.h>

struct InternalData2
{
	HWND m_hWnd;
	;
	i32 m_fullWindowWidth;  //includes borders etc
	i32 m_fullWindowHeight;

	i32 m_openglViewportWidth;  //just the 3d viewport/client area
	i32 m_openglViewportHeight;

	HDC m_hDC;
	HGLRC m_hRC;
	bool m_OpenGLInitialized;
	i32 m_oldScreenWidth;
	i32 m_oldHeight;
	i32 m_oldBitsPerPel;
	bool m_quit;
	i32 m_mouseLButton;
	i32 m_mouseRButton;
	i32 m_mouseMButton;
	i32 m_mouseXpos;
	i32 m_mouseYpos;

	i32 m_internalKeyModifierFlags;

	b3WheelCallback m_wheelCallback;
	b3MouseMoveCallback m_mouseMoveCallback;
	b3MouseButtonCallback m_mouseButtonCallback;
	b3ResizeCallback m_resizeCallback;
	b3KeyboardCallback m_keyboardCallback;

	InternalData2()
	{
		m_hWnd = 0;
		m_mouseLButton = 0;
		m_mouseRButton = 0;
		m_mouseMButton = 0;
		m_internalKeyModifierFlags = 0;
		m_fullWindowWidth = 0;
		m_fullWindowHeight = 0;
		m_openglViewportHeight = 0;
		m_openglViewportWidth = 0;
		m_hDC = 0;
		m_hRC = 0;
		m_OpenGLInitialized = false;
		m_oldScreenWidth = 0;
		m_oldHeight = 0;
		m_oldBitsPerPel = 0;
		m_quit = false;

		m_keyboardCallback = 0;
		m_mouseMoveCallback = 0;
		m_mouseButtonCallback = 0;
		m_resizeCallback = 0;
		m_wheelCallback = 0;
	}
};

#endif  //WIN32_INTERNAL_WINDOW_DATA_H