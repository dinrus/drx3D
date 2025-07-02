#ifndef D3_USE_GLFW
#ifdef _WIN32

#include "Win32OpenGLWindow.h"
#include "OpenGLInclude.h"
#include "Win32InternalWindowData.h"
#include <stdio.h>
#include <stdlib.h>

void Win32OpenGLWindow::enableOpenGL()
{
	PIXELFORMATDESCRIPTOR pfd;
	i32 format;

	// get the device context (DC)
	m_data->m_hDC = GetDC(m_data->m_hWnd);

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;  //1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(m_data->m_hDC, &pfd);
	SetPixelFormat(m_data->m_hDC, format, &pfd);

	// create and enable the render context (RC)
	m_data->m_hRC = wglCreateContext(m_data->m_hDC);
	wglMakeCurrent(m_data->m_hDC, m_data->m_hRC);

	//printGLString("Extensions", GL_EXTENSIONS);
}

void Win32OpenGLWindow::disableOpenGL()
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(m_data->m_hRC);
	//	ReleaseDC( m_data->m_hWnd, m_data->m_hDC );
}

void Win32OpenGLWindow::createWindow(const b3gWindowConstructionInfo& ci)
{
	Win32Window::createWindow(ci);

	//VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, this);
	enableOpenGL();

	if (!gladLoaderLoadGL())
	{
		printf("Не сработал gladLoaderLoadGL!\n");
		exit(-1);
	}
}

Win32OpenGLWindow::Win32OpenGLWindow()
{
}

Win32OpenGLWindow::~Win32OpenGLWindow()
{
}

void Win32OpenGLWindow::closeWindow()
{
	disableOpenGL();

	Win32Window::closeWindow();
}

void Win32OpenGLWindow::startRendering()
{
	pumpMessage();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
}

void Win32OpenGLWindow::renderAllObjects()
{
}

void Win32OpenGLWindow::endRendering()
{
	SwapBuffers(m_data->m_hDC);
}

i32 Win32OpenGLWindow::fileOpenDialog(tuk fileName, i32 maxFileNameLength)
{
#if 0
	//wchar_t wideChars[1024];

		OPENFILENAME ofn ;
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL  ;

#ifdef UNICODE
	WCHAR bla[1024];
	ofn.lpstrFile = bla;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 1023;
	ofn.lpstrFilter = L"Все Файлы\0*.*\0URDF\0*.urdf\0.bullet\0*.bullet\0";
#else
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 1023;
	//ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	ofn.lpstrFilter = "All Files\0*.*\0URDF\0*.urdf\0.bullet\0*.bullet\0";

#endif

	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir=NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ;
	GetOpenFileName( &ofn );
	return strlen(fileName);
#else
	return 0;
#endif
}

i32 Win32OpenGLWindow::getWidth() const
{
	if (m_data)
		return m_data->m_openglViewportWidth;
	return 0;
}

i32 Win32OpenGLWindow::getHeight() const
{
	if (m_data)
		return m_data->m_openglViewportHeight;
	return 0;
}

#endif

#endif  //D3_USE_GLFW
