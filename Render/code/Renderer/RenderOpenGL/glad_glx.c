// THIS FILE IS AUTOMATICALLY GENERATED
// Please do not modify directly.
// For information on how to regenerate or extend
// see: /Code/Tools/glad/README

#include <stdio.h>
#include <string.h>
#include <drx3D/Render/glad_glx.h>

static uk get_proc(tukk namez);

#ifdef _WIN32
#include <windows.h>
static HMODULE libGL;

typedef uk (APIENTRYP PFNWGLGETPROCADDRESSPROC_PRIVATE)(tukk);
PFNWGLGETPROCADDRESSPROC_PRIVATE gladGetProcAddressPtr;

static
i32 open_gl(void) {
    libGL = LoadLibraryA("opengl32.dll");
    if(libGL != NULL) {
        gladGetProcAddressPtr = (PFNWGLGETPROCADDRESSPROC_PRIVATE)GetProcAddress(
                libGL, "wglGetProcAddress");
        return gladGetProcAddressPtr != NULL;
    }

    return 0;
}

static
void close_gl(void) {
    if(libGL != NULL) {
        FreeLibrary(libGL);
        libGL = NULL;
    }
}
#else
#include <dlfcn.h>
static uk libGL;

#ifndef __APPLE__
typedef uk (APIENTRYP PFNGLXGETPROCADDRESSPROC_PRIVATE)(tukk);
PFNGLXGETPROCADDRESSPROC_PRIVATE gladGetProcAddressPtr;
#endif

static
i32 open_gl(void) {
#ifdef __APPLE__
    static tukk NAMES[] = {
        "../Frameworks/OpenGL.framework/OpenGL",
        "/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL"
    };
#elif defined(__ANDROID__)
    static tukk NAMES[] = {"libEGL.so"};
#else
    static tukk NAMES[] = {"libGL.so.1", "libGL.so"};
#endif

    u32 index = 0;
    for(index = 0; index < (sizeof(NAMES) / sizeof(NAMES[0])); index++) {
        libGL = dlopen(NAMES[index], RTLD_NOW | RTLD_GLOBAL);

        if(libGL != NULL) {
#ifdef __APPLE__
            return 1;
#elif defined(__ANDROID__)
            gladGetProcAddressPtr = (PFNGLXGETPROCADDRESSPROC_PRIVATE)dlsym(libGL,
                "eglGetProcAddress");
            return gladGetProcAddressPtr != NULL;
#else
            gladGetProcAddressPtr = (PFNGLXGETPROCADDRESSPROC_PRIVATE)dlsym(libGL,
                "glXGetProcAddressARB");
            return gladGetProcAddressPtr != NULL;
#endif
        }
    }

    return 0;
}

static
void close_gl() {
    if(libGL != NULL) {
        dlclose(libGL);
        libGL = NULL;
    }
}
#endif

static
uk get_proc(tukk namez) {
    uk result = NULL;
    if(libGL == NULL) return NULL;

#ifndef __APPLE__
    if(gladGetProcAddressPtr != NULL) {
        result = gladGetProcAddressPtr(namez);
    }
#endif
    if(result == NULL) {
#ifdef _WIN32
        result = (uk )GetProcAddress(libGL, namez);
#else
        result = dlsym(libGL, namez);
#endif
    }

    return result;
}

i32 gladLoadGLX(Display *dpy, i32 screen) {
    i32 status = 0;

    if(open_gl()) {
        status = gladLoadGLXLoader((GLADloadproc)get_proc, dpy, screen);
        close_gl();
    }

    return status;
}

static Display *GLADGLXDisplay = 0;
static i32 GLADGLXscreen = 0;

static i32 has_ext(tukk ext) {
    tukk terminator;
    tukk loc;
    tukk extensions;

    if(!GLAD_GLX_VERSION_1_1)
        return 0;

    extensions = glXQueryExtensionsString(GLADGLXDisplay, GLADGLXscreen);

    if(extensions == NULL || ext == NULL)
        return 0;

    while(1) {
        loc = strstr(extensions, ext);
        if(loc == NULL)
            break;

        terminator = loc + strlen(ext);
        if((loc == extensions || *(loc - 1) == ' ') &&
            (*terminator == ' ' || *terminator == '\0'))
        {
            return 1;
        }
        extensions = terminator;
    }

    return 0;
}

i32 GLAD_GLX_VERSION_1_0;
i32 GLAD_GLX_VERSION_1_1;
i32 GLAD_GLX_VERSION_1_2;
i32 GLAD_GLX_VERSION_1_3;
i32 GLAD_GLX_VERSION_1_4;
PFNGLXGETSELECTEDEVENTPROC glad_glXGetSelectedEvent;
PFNGLXQUERYEXTENSIONPROC glad_glXQueryExtension;
PFNGLXMAKECURRENTPROC glad_glXMakeCurrent;
PFNGLXSELECTEVENTPROC glad_glXSelectEvent;
PFNGLXCREATECONTEXTPROC glad_glXCreateContext;
PFNGLXCREATEGLXPIXMAPPROC glad_glXCreateGLXPixmap;
PFNGLXQUERYVERSIONPROC glad_glXQueryVersion;
PFNGLXGETCURRENTREADDRAWABLEPROC glad_glXGetCurrentReadDrawable;
PFNGLXDESTROYPIXMAPPROC glad_glXDestroyPixmap;
PFNGLXGETCURRENTCONTEXTPROC glad_glXGetCurrentContext;
PFNGLXGETPROCADDRESSPROC glad_glXGetProcAddress;
PFNGLXWAITGLPROC glad_glXWaitGL;
PFNGLXISDIRECTPROC glad_glXIsDirect;
PFNGLXDESTROYWINDOWPROC glad_glXDestroyWindow;
PFNGLXCREATEWINDOWPROC glad_glXCreateWindow;
PFNGLXCOPYCONTEXTPROC glad_glXCopyContext;
PFNGLXCREATEPBUFFERPROC glad_glXCreatePbuffer;
PFNGLXSWAPBUFFERSPROC glad_glXSwapBuffers;
PFNGLXGETCURRENTDISPLAYPROC glad_glXGetCurrentDisplay;
PFNGLXGETCURRENTDRAWABLEPROC glad_glXGetCurrentDrawable;
PFNGLXQUERYCONTEXTPROC glad_glXQueryContext;
PFNGLXCHOOSEVISUALPROC glad_glXChooseVisual;
PFNGLXQUERYSERVERSTRINGPROC glad_glXQueryServerString;
PFNGLXDESTROYCONTEXTPROC glad_glXDestroyContext;
PFNGLXDESTROYGLXPIXMAPPROC glad_glXDestroyGLXPixmap;
PFNGLXGETFBCONFIGATTRIBPROC glad_glXGetFBConfigAttrib;
PFNGLXUSEXFONTPROC glad_glXUseXFont;
PFNGLXDESTROYPBUFFERPROC glad_glXDestroyPbuffer;
PFNGLXCHOOSEFBCONFIGPROC glad_glXChooseFBConfig;
PFNGLXCREATENEWCONTEXTPROC glad_glXCreateNewContext;
PFNGLXMAKECONTEXTCURRENTPROC glad_glXMakeContextCurrent;
PFNGLXGETCONFIGPROC glad_glXGetConfig;
PFNGLXGETFBCONFIGSPROC glad_glXGetFBConfigs;
PFNGLXCREATEPIXMAPPROC glad_glXCreatePixmap;
PFNGLXWAITXPROC glad_glXWaitX;
PFNGLXGETVISUALFROMFBCONFIGPROC glad_glXGetVisualFromFBConfig;
PFNGLXQUERYDRAWABLEPROC glad_glXQueryDrawable;
PFNGLXQUERYEXTENSIONSSTRINGPROC glad_glXQueryExtensionsString;
PFNGLXGETCLIENTSTRINGPROC glad_glXGetClientString;
static void load_GLX_VERSION_1_0(GLADloadproc load) {
	if(!GLAD_GLX_VERSION_1_0) return;
	glad_glXChooseVisual = (PFNGLXCHOOSEVISUALPROC)load("glXChooseVisual");
	glad_glXCreateContext = (PFNGLXCREATECONTEXTPROC)load("glXCreateContext");
	glad_glXDestroyContext = (PFNGLXDESTROYCONTEXTPROC)load("glXDestroyContext");
	glad_glXMakeCurrent = (PFNGLXMAKECURRENTPROC)load("glXMakeCurrent");
	glad_glXCopyContext = (PFNGLXCOPYCONTEXTPROC)load("glXCopyContext");
	glad_glXSwapBuffers = (PFNGLXSWAPBUFFERSPROC)load("glXSwapBuffers");
	glad_glXCreateGLXPixmap = (PFNGLXCREATEGLXPIXMAPPROC)load("glXCreateGLXPixmap");
	glad_glXDestroyGLXPixmap = (PFNGLXDESTROYGLXPIXMAPPROC)load("glXDestroyGLXPixmap");
	glad_glXQueryExtension = (PFNGLXQUERYEXTENSIONPROC)load("glXQueryExtension");
	glad_glXQueryVersion = (PFNGLXQUERYVERSIONPROC)load("glXQueryVersion");
	glad_glXIsDirect = (PFNGLXISDIRECTPROC)load("glXIsDirect");
	glad_glXGetConfig = (PFNGLXGETCONFIGPROC)load("glXGetConfig");
	glad_glXGetCurrentContext = (PFNGLXGETCURRENTCONTEXTPROC)load("glXGetCurrentContext");
	glad_glXGetCurrentDrawable = (PFNGLXGETCURRENTDRAWABLEPROC)load("glXGetCurrentDrawable");
	glad_glXWaitGL = (PFNGLXWAITGLPROC)load("glXWaitGL");
	glad_glXWaitX = (PFNGLXWAITXPROC)load("glXWaitX");
	glad_glXUseXFont = (PFNGLXUSEXFONTPROC)load("glXUseXFont");
}
static void load_GLX_VERSION_1_1(GLADloadproc load) {
	if(!GLAD_GLX_VERSION_1_1) return;
	glad_glXQueryExtensionsString = (PFNGLXQUERYEXTENSIONSSTRINGPROC)load("glXQueryExtensionsString");
	glad_glXQueryServerString = (PFNGLXQUERYSERVERSTRINGPROC)load("glXQueryServerString");
	glad_glXGetClientString = (PFNGLXGETCLIENTSTRINGPROC)load("glXGetClientString");
}
static void load_GLX_VERSION_1_2(GLADloadproc load) {
	if(!GLAD_GLX_VERSION_1_2) return;
	glad_glXGetCurrentDisplay = (PFNGLXGETCURRENTDISPLAYPROC)load("glXGetCurrentDisplay");
}
static void load_GLX_VERSION_1_3(GLADloadproc load) {
	if(!GLAD_GLX_VERSION_1_3) return;
	glad_glXGetFBConfigs = (PFNGLXGETFBCONFIGSPROC)load("glXGetFBConfigs");
	glad_glXChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)load("glXChooseFBConfig");
	glad_glXGetFBConfigAttrib = (PFNGLXGETFBCONFIGATTRIBPROC)load("glXGetFBConfigAttrib");
	glad_glXGetVisualFromFBConfig = (PFNGLXGETVISUALFROMFBCONFIGPROC)load("glXGetVisualFromFBConfig");
	glad_glXCreateWindow = (PFNGLXCREATEWINDOWPROC)load("glXCreateWindow");
	glad_glXDestroyWindow = (PFNGLXDESTROYWINDOWPROC)load("glXDestroyWindow");
	glad_glXCreatePixmap = (PFNGLXCREATEPIXMAPPROC)load("glXCreatePixmap");
	glad_glXDestroyPixmap = (PFNGLXDESTROYPIXMAPPROC)load("glXDestroyPixmap");
	glad_glXCreatePbuffer = (PFNGLXCREATEPBUFFERPROC)load("glXCreatePbuffer");
	glad_glXDestroyPbuffer = (PFNGLXDESTROYPBUFFERPROC)load("glXDestroyPbuffer");
	glad_glXQueryDrawable = (PFNGLXQUERYDRAWABLEPROC)load("glXQueryDrawable");
	glad_glXCreateNewContext = (PFNGLXCREATENEWCONTEXTPROC)load("glXCreateNewContext");
	glad_glXMakeContextCurrent = (PFNGLXMAKECONTEXTCURRENTPROC)load("glXMakeContextCurrent");
	glad_glXGetCurrentReadDrawable = (PFNGLXGETCURRENTREADDRAWABLEPROC)load("glXGetCurrentReadDrawable");
	glad_glXQueryContext = (PFNGLXQUERYCONTEXTPROC)load("glXQueryContext");
	glad_glXSelectEvent = (PFNGLXSELECTEVENTPROC)load("glXSelectEvent");
	glad_glXGetSelectedEvent = (PFNGLXGETSELECTEDEVENTPROC)load("glXGetSelectedEvent");
}
static void load_GLX_VERSION_1_4(GLADloadproc load) {
	if(!GLAD_GLX_VERSION_1_4) return;
	glad_glXGetProcAddress = (PFNGLXGETPROCADDRESSPROC)load("glXGetProcAddress");
}
static void find_extensionsGLX(void) {
}

static void find_coreGLX(Display *dpy, i32 screen) {
	i32 major = 0, minor = 0;
	if(dpy == 0 && GLADGLXDisplay == 0) {
		dpy = XOpenDisplay(0);
		screen = XScreenNumberOfScreen(XDefaultScreenOfDisplay(dpy));
	} else if(dpy == 0) {
		dpy = GLADGLXDisplay;
		screen = GLADGLXscreen;
	}
	glXQueryVersion(dpy, &major, &minor);
	GLADGLXDisplay = dpy;
	GLADGLXscreen = screen;
	GLAD_GLX_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
	GLAD_GLX_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
	GLAD_GLX_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
	GLAD_GLX_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
	GLAD_GLX_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
}

i32 gladLoadGLXLoader(GLADloadproc load, Display *dpy, i32 screen) {
	glXQueryVersion = (PFNGLXQUERYVERSIONPROC)load("glXQueryVersion");
	if(glXQueryVersion == NULL) return 0;
	find_coreGLX(dpy, screen);
	load_GLX_VERSION_1_0(load);
	load_GLX_VERSION_1_1(load);
	load_GLX_VERSION_1_2(load);
	load_GLX_VERSION_1_3(load);
	load_GLX_VERSION_1_4(load);

	find_extensionsGLX();
	return 1;
}

