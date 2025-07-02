#ifndef __OPENGL_INCLUDE_H
#define __OPENGL_INCLUDE_H

#ifdef DRX3D_NO_GLAD
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GL/gl.h"
#else
#define D3_USE_GLFW
#include <X/glfw/glad/gl.h>
#include <X/glfw/glfw3.h>
#include <GL/glcorearb.h>
#endif  //DRX3D_NO_GLAD

#endif  //__OPENGL_INCLUDE_H
