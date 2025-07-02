// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __glad_egl_h_

#ifdef __egl_h_
	#error EGL header already included, remove this include, glad already provides it
#endif

#define __glad_egl_h_
#define __egl_h_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN 1
	#endif
	#include <windows.h>
#endif

#ifndef APIENTRY
	#define APIENTRY
#endif
#ifndef APIENTRYP
	#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
	#define GLAPI extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __gladloadproc__
	#define __gladloadproc__
typedef uk (* GLADloadproc)(tukk name);
#endif
GLAPI i32 gladLoadEGLLoader(GLADloadproc);

GLAPI i32 gladLoadEGL(void);
GLAPI i32 gladLoadEGLLoader(GLADloadproc);

#include <drx3D/Render/khrplatform.h>
#include <EGL/eglplatform.h>
typedef u32                EGLBoolean;
typedef u32                EGLenum;
typedef intptr_t                    EGLAttribKHR;
typedef intptr_t                    EGLAttrib;
typedef uk                       EGLClientBuffer;
typedef uk                       EGLConfig;
typedef uk                       EGLContext;
typedef uk                       EGLDeviceEXT;
typedef uk                       EGLDisplay;
typedef uk                       EGLImage;
typedef uk                       EGLImageKHR;
typedef uk                       EGLOutputLayerEXT;
typedef uk                       EGLOutputPortEXT;
typedef uk                       EGLStreamKHR;
typedef uk                       EGLSurface;
typedef uk                       EGLSync;
typedef uk                       EGLSyncKHR;
typedef uk                       EGLSyncNV;
typedef void (*                     __eglMustCastToProperFunctionPointerType)(void);
typedef khronos_utime_nanoseconds_t EGLTimeKHR;
typedef khronos_utime_nanoseconds_t EGLTime;
typedef khronos_utime_nanoseconds_t EGLTimeNV;
typedef khronos_utime_nanoseconds_t EGLuint64NV;
typedef khronos_uint64_t            EGLuint64KHR;
typedef i32                         EGLNativeFileDescriptorKHR;
typedef khronos_ssize_t             EGLsizeiANDROID;
typedef void (*                     EGLSetBlobFuncANDROID) (ukk key, EGLsizeiANDROID keySize, ukk value, EGLsizeiANDROID valueSize);
typedef EGLsizeiANDROID (*          EGLGetBlobFuncANDROID) (ukk key, EGLsizeiANDROID keySize, uk value, EGLsizeiANDROID valueSize);
struct EGLClientPixmapHI
{
	uk  pData;
	EGLint iWidth;
	EGLint iHeight;
	EGLint iStride;
};
EGLBoolean eglChooseConfig(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
EGLBoolean eglCopyBuffers(EGLDisplay, EGLSurface, EGLNativePixmapType);
EGLContext eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
EGLSurface eglCreatePbufferSurface(EGLDisplay, EGLConfig, const EGLint*);
EGLSurface eglCreatePixmapSurface(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint*);
EGLSurface eglCreateWindowSurface(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
EGLBoolean eglDestroyContext(EGLDisplay, EGLContext);
EGLBoolean eglDestroySurface(EGLDisplay, EGLSurface);
EGLBoolean eglGetConfigAttrib(EGLDisplay, EGLConfig, EGLint, EGLint*);
EGLBoolean eglGetConfigs(EGLDisplay, EGLConfig*, EGLint, EGLint*);
EGLDisplay eglGetCurrentDisplay();
EGLSurface eglGetCurrentSurface(EGLint);
EGLDisplay eglGetDisplay(EGLNativeDisplayType);
EGLint                                   eglGetError();
__eglMustCastToProperFunctionPointerType eglGetProcAddress(tukk);
EGLBoolean eglInitialize(EGLDisplay, EGLint*, EGLint*);
EGLBoolean eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
EGLBoolean eglQueryContext(EGLDisplay, EGLContext, EGLint, EGLint*);
tukk eglQueryString(EGLDisplay, EGLint);
EGLBoolean eglQuerySurface(EGLDisplay, EGLSurface, EGLint, EGLint*);
EGLBoolean eglSwapBuffers(EGLDisplay, EGLSurface);
EGLBoolean eglTerminate(EGLDisplay);
EGLBoolean eglWaitGL();
EGLBoolean eglWaitNative(EGLint);
EGLBoolean eglBindTexImage(EGLDisplay, EGLSurface, EGLint);
EGLBoolean eglReleaseTexImage(EGLDisplay, EGLSurface, EGLint);
EGLBoolean eglSurfaceAttrib(EGLDisplay, EGLSurface, EGLint, EGLint);
EGLBoolean eglSwapInterval(EGLDisplay, EGLint);
EGLBoolean eglBindAPI(EGLenum);
EGLenum    eglQueryAPI();
EGLSurface eglCreatePbufferFromClientBuffer(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint*);
EGLBoolean eglReleaseThread();
EGLBoolean eglWaitClient();
EGLContext eglGetCurrentContext();
EGLSync eglCreateSync(EGLDisplay, EGLenum, const EGLAttrib*);
EGLBoolean eglDestroySync(EGLDisplay, EGLSync);
EGLint eglClientWaitSync(EGLDisplay, EGLSync, EGLint, EGLTime);
EGLBoolean eglGetSyncAttrib(EGLDisplay, EGLSync, EGLint, EGLAttrib*);
EGLImage eglCreateImage(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLAttrib*);
EGLBoolean eglDestroyImage(EGLDisplay, EGLImage);
EGLDisplay eglGetPlatformDisplay(EGLenum, uk , const EGLAttrib*);
EGLSurface eglCreatePlatformWindowSurface(EGLDisplay, EGLConfig, uk , const EGLAttrib*);
EGLSurface eglCreatePlatformPixmapSurface(EGLDisplay, EGLConfig, uk , const EGLAttrib*);
EGLBoolean eglWaitSync(EGLDisplay, EGLSync, EGLint);

#ifdef __cplusplus
}
#endif

#endif
