#ifndef MAC_OPENGL_WINDOW_OBJC_H
#define MAC_OPENGL_WINDOW_OBJC_H

struct MacOpenGLWindowInternalData;

#include <drx3D/Common/Interfaces/CommonCallbacks.h"

struct MacWindowConstructionInfo
{
	i32 m_width;
	i32 m_height;
	i32 m_fullscreen;
	i32 m_colorBitsPerPixel;
	uk m_windowHandle;
	tukk m_title;
	i32 m_openglVersion;
	i32 m_allowRetina;
};

enum
{
	MY_MAC_ALTKEY = 1,
	MY_MAC_SHIFTKEY = 2,
	MY_MAC_CONTROL_KEY = 4
};

#ifdef __cplusplus
extern "C"
{
#endif

	struct MacOpenGLWindowInternalData* Mac_createData();
	void Mac_destroyData(struct MacOpenGLWindowInternalData* data);

	i32 Mac_createWindow(struct MacOpenGLWindowInternalData* m_internalData, struct MacWindowConstructionInfo* ci);

	void Mac_setWindowTitle(struct MacOpenGLWindowInternalData* data, tukk windowTitle);
	i32 Mac_updateWindow(struct MacOpenGLWindowInternalData* m_internalData);
	void Mac_swapBuffer(struct MacOpenGLWindowInternalData* m_internalData);
	i32 Mac_requestedExit(struct MacOpenGLWindowInternalData* m_internalData);
	void Mac_setRequestExit(struct MacOpenGLWindowInternalData* m_internalData);
	float Mac_getRetinaScale(struct MacOpenGLWindowInternalData* m_internalData);
	void Mac_setAllowRetina(struct MacOpenGLWindowInternalData* m_internalData, i32 allow);

	i32 Mac_getWidth(struct MacOpenGLWindowInternalData* m_internalData);
	i32 Mac_getHeight(struct MacOpenGLWindowInternalData* m_internalData);

	i32 Mac_fileOpenDialog(tuk filename, i32 maxNameLength);

	void Mac_setKeyboardCallback(struct MacOpenGLWindowInternalData* m_internalData, b3KeyboardCallback keyboardCallback);
	b3KeyboardCallback Mac_getKeyboardCallback(struct MacOpenGLWindowInternalData* m_internalData);
	i32 Mac_isModifierKeyPressed(struct MacOpenGLWindowInternalData* m_internalData, i32 key);

	void Mac_setMouseButtonCallback(struct MacOpenGLWindowInternalData* m_internalData, b3MouseButtonCallback mouseCallback);
	b3MouseButtonCallback Mac_getMouseButtonCallback(struct MacOpenGLWindowInternalData* m_internalData);
	void Mac_getMouseCoordinates(struct MacOpenGLWindowInternalData* m_internalData, i32* xPtr, i32* yPtr);
	void Mac_setMouseMoveCallback(struct MacOpenGLWindowInternalData* m_internalData, b3MouseMoveCallback mouseCallback);
	b3MouseMoveCallback Mac_getMouseMoveCallback(struct MacOpenGLWindowInternalData* m_internalData);

	void Mac_setWheelCallback(struct MacOpenGLWindowInternalData* m_internalData, b3WheelCallback wheelCallback);
	b3WheelCallback Mac_getWheelCallback(struct MacOpenGLWindowInternalData* m_internalData);

	void Mac_setResizeCallback(struct MacOpenGLWindowInternalData* m_internalData, b3ResizeCallback resizeCallback);
	b3ResizeCallback Mac_getResizeCallback(struct MacOpenGLWindowInternalData* m_internalData);

	//void Mac_setRenderCallback(struct MacOpenGLWindowInternalData* m_internalData, b3RenderCallback renderCallback);

#ifdef __cplusplus
}
#endif

#endif  //MAC_OPENGL_WINDOW_OBJC_H
