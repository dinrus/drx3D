#ifndef __OPENGL_INCLUDE_H
#define __OPENGL_INCLUDE_H
/*
#ifdef DRX3D_NO_GLAD
#include "third_party/GL/gl/include/EGL/egl.h"
#include "third_party/GL/gl/include/EGL/eglext.h"
#include "third_party/GL/gl/include/GL/gl.h"
#else
*/
#define D3_USE_GLFW
#include <X/glfw/glad/gl.h>
#include <X/glfw/glfw3.h>
//#else
//#include "glad/gl.h"
//#endif  //D3_USE_GLFW
//#endif  //DRX3D_NO_GLAD

//disable glGetError
//#undef glGetError
//#define glGetError MyGetError
//
//GLenum inline MyGetError()
//{
//	return 0;
//}

///on Linux only glDrawElementsInstancedARB is defined?!?
//#ifdef __linux
//#define glDrawElementsInstanced glDrawElementsInstancedARB
//
//#endif //__linux

#endif  //__OPENGL_INCLUDE_H
