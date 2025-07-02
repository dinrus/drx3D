// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/NULLRenderAuxGeom.h>
#include <drx3D/CoreX/Platform/DrxLibrary.h>

CNULLRenderAuxGeom* CNULLRenderAuxGeom::s_pThis = NULL;

#ifdef ENABLE_WGL_DEBUG_RENDERER

static const float PI = 3.14159265358979323f;
static const float W = 800.0f;
static const float H = 600.0f;
static const float THETA = 5.0f;
static const Vec3 VUP(0.0f, 0.0f, 1.0f);

bool CNULLRenderAuxGeom::s_active = false;
bool CNULLRenderAuxGeom::s_hidden = true;

	#ifdef wglUseFontBitmaps
		#undef wglUseFontBitmaps
	#endif

//////////////////////////////////////////////////////////////////////////
// Dynamically load OpenGL.
//////////////////////////////////////////////////////////////////////////

namespace drx_opengl
{
HGLRC(WINAPI * drx_wglCreateContext)(HDC);
BOOL(WINAPI * drx_wglDeleteContext)(HGLRC);
BOOL(WINAPI * drx_wglMakeCurrent)(HDC, HGLRC);
BOOL(WINAPI * drx_wglUseFontBitmaps)(HDC, DWORD, DWORD, DWORD);

void(APIENTRY * drx_gluPerspective)(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
void(APIENTRY * drx_gluLookAt)(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
i32(APIENTRY * drx_gluProject) (GLdouble objx, GLdouble objy, GLdouble objz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble * winx, GLdouble * winy, GLdouble * winz);
void(APIENTRY * drx_gluSphere) (GLUquadric * qobj, GLdouble radius, GLint slices, GLint stacks);
GLUquadric* (APIENTRY * drx_gluNewQuadric)(void);
void(APIENTRY * drx_gluDeleteQuadric) (GLUquadric * state);

void(APIENTRY * drx_glAccum)(GLenum op, GLfloat value);
void(APIENTRY * drx_glAlphaFunc)(GLenum func, GLclampf ref);
GLboolean(APIENTRY * drx_glAreTexturesResident)(GLsizei n, const GLuint * textures, GLboolean * residences);
void(APIENTRY * drx_glArrayElement)(GLint i);
void(APIENTRY * drx_glBegin)(GLenum mode);
void(APIENTRY * drx_glBindTexture)(GLenum target, GLuint texture);
void(APIENTRY * drx_glBitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap);
void(APIENTRY * drx_glBlendFunc)(GLenum sfactor, GLenum dfactor);
void(APIENTRY * drx_glCallList)(GLuint list);
void(APIENTRY * drx_glCallLists)(GLsizei n, GLenum type, const GLvoid * lists);
void(APIENTRY * drx_glClear)(GLbitfield mask);
void(APIENTRY * drx_glClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void(APIENTRY * drx_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void(APIENTRY * drx_glClearDepth)(GLclampd depth);
void(APIENTRY * drx_glClearIndex)(GLfloat c);
void(APIENTRY * drx_glClearStencil)(GLint s);
void(APIENTRY * drx_glClipPlane)(GLenum plane, const GLdouble * equation);
void(APIENTRY * drx_glColor3b)(GLbyte red, GLbyte green, GLbyte blue);
void(APIENTRY * drx_glColor3bv)(const GLbyte * v);
void(APIENTRY * drx_glColor3d)(GLdouble red, GLdouble green, GLdouble blue);
void(APIENTRY * drx_glColor3dv)(const GLdouble * v);
void(APIENTRY * drx_glColor3f)(GLfloat red, GLfloat green, GLfloat blue);
void(APIENTRY * drx_glColor3fv)(const GLfloat * v);
void(APIENTRY * drx_glColor3i)(GLint red, GLint green, GLint blue);
void(APIENTRY * drx_glColor3iv)(const GLint * v);
void(APIENTRY * drx_glColor3s)(GLshort red, GLshort green, GLshort blue);
void(APIENTRY * drx_glColor3sv)(const GLshort * v);
void(APIENTRY * drx_glColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
void(APIENTRY * drx_glColor3ubv)(const GLubyte * v);
void(APIENTRY * drx_glColor3ui)(GLuint red, GLuint green, GLuint blue);
void(APIENTRY * drx_glColor3uiv)(const GLuint * v);
void(APIENTRY * drx_glColor3us)(GLushort red, GLushort green, GLushort blue);
void(APIENTRY * drx_glColor3usv)(const GLushort * v);
void(APIENTRY * drx_glColor4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void(APIENTRY * drx_glColor4bv)(const GLbyte * v);
void(APIENTRY * drx_glColor4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void(APIENTRY * drx_glColor4dv)(const GLdouble * v);
void(APIENTRY * drx_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void(APIENTRY * drx_glColor4fv)(const GLfloat * v);
void(APIENTRY * drx_glColor4i)(GLint red, GLint green, GLint blue, GLint alpha);
void(APIENTRY * drx_glColor4iv)(const GLint * v);
void(APIENTRY * drx_glColor4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
void(APIENTRY * drx_glColor4sv)(const GLshort * v);
void(APIENTRY * drx_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void(APIENTRY * drx_glColor4ubv)(const GLubyte * v);
void(APIENTRY * drx_glColor4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
void(APIENTRY * drx_glColor4uiv)(const GLuint * v);
void(APIENTRY * drx_glColor4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void(APIENTRY * drx_glColor4usv)(const GLushort * v);
void(APIENTRY * drx_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void(APIENTRY * drx_glColorMaterial)(GLenum face, GLenum mode);
void(APIENTRY * drx_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glCopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void(APIENTRY * drx_glCopyTexImage1D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void(APIENTRY * drx_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void(APIENTRY * drx_glCopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void(APIENTRY * drx_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void(APIENTRY * drx_glCullFace)(GLenum mode);
void(APIENTRY * drx_glDeleteLists)(GLuint list, GLsizei range);
void(APIENTRY * drx_glDeleteTextures)(GLsizei n, const GLuint * textures);
void(APIENTRY * drx_glDepthFunc)(GLenum func);
void(APIENTRY * drx_glDepthMask)(GLboolean flag);
void(APIENTRY * drx_glDepthRange)(GLclampd zNear, GLclampd zFar);
void(APIENTRY * drx_glDisable)(GLenum cap);
void(APIENTRY * drx_glDisableClientState)(GLenum array);
void(APIENTRY * drx_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
void(APIENTRY * drx_glDrawBuffer)(GLenum mode);
void(APIENTRY * drx_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
void(APIENTRY * drx_glDrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
void(APIENTRY * drx_glEdgeFlag)(GLboolean flag);
void(APIENTRY * drx_glEdgeFlagPointer)(GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glEdgeFlagv)(const GLboolean * flag);
void(APIENTRY * drx_glEnable)(GLenum cap);
void(APIENTRY * drx_glEnableClientState)(GLenum array);
void(APIENTRY * drx_glEnd)(void);
void(APIENTRY * drx_glEndList)(void);
void(APIENTRY * drx_glEvalCoord1d)(GLdouble u);
void(APIENTRY * drx_glEvalCoord1dv)(const GLdouble * u);
void(APIENTRY * drx_glEvalCoord1f)(GLfloat u);
void(APIENTRY * drx_glEvalCoord1fv)(const GLfloat * u);
void(APIENTRY * drx_glEvalCoord2d)(GLdouble u, GLdouble v);
void(APIENTRY * drx_glEvalCoord2dv)(const GLdouble * u);
void(APIENTRY * drx_glEvalCoord2f)(GLfloat u, GLfloat v);
void(APIENTRY * drx_glEvalCoord2fv)(const GLfloat * u);
void(APIENTRY * drx_glEvalMesh1)(GLenum mode, GLint i1, GLint i2);
void(APIENTRY * drx_glEvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void(APIENTRY * drx_glEvalPoint1)(GLint i);
void(APIENTRY * drx_glEvalPoint2)(GLint i, GLint j);
void(APIENTRY * drx_glFeedbackBuffer)(GLsizei size, GLenum type, GLfloat * buffer);
void(APIENTRY * drx_glFinish)(void);
void(APIENTRY * drx_glFlush)(void);
void(APIENTRY * drx_glFogf)(GLenum pname, GLfloat param);
void(APIENTRY * drx_glFogfv)(GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glFogi)(GLenum pname, GLint param);
void(APIENTRY * drx_glFogiv)(GLenum pname, const GLint * params);
void(APIENTRY * drx_glFrontFace)(GLenum mode);
void(APIENTRY * drx_glFrustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint(APIENTRY * drx_glGenLists)(GLsizei range);
void(APIENTRY * drx_glGenTextures)(GLsizei n, GLuint * textures);
void(APIENTRY * drx_glGetBooleanv)(GLenum pname, GLboolean * params);
void(APIENTRY * drx_glGetClipPlane)(GLenum plane, GLdouble * equation);
void(APIENTRY * drx_glGetDoublev)(GLenum pname, GLdouble * params);
GLenum(APIENTRY * drx_glGetError)(void);
void(APIENTRY * drx_glGetFloatv)(GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetIntegerv)(GLenum pname, GLint * params);
void(APIENTRY * drx_glGetLightfv)(GLenum light, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetLightiv)(GLenum light, GLenum pname, GLint * params);
void(APIENTRY * drx_glGetMapdv)(GLenum target, GLenum query, GLdouble * v);
void(APIENTRY * drx_glGetMapfv)(GLenum target, GLenum query, GLfloat * v);
void(APIENTRY * drx_glGetMapiv)(GLenum target, GLenum query, GLint * v);
void(APIENTRY * drx_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetMaterialiv)(GLenum face, GLenum pname, GLint * params);
void(APIENTRY * drx_glGetPixelMapfv)(GLenum map, GLfloat * values);
void(APIENTRY * drx_glGetPixelMapuiv)(GLenum map, GLuint * values);
void(APIENTRY * drx_glGetPixelMapusv)(GLenum map, GLushort * values);
void(APIENTRY * drx_glGetPointerv)(GLenum pname, GLvoid * *params);
void(APIENTRY * drx_glGetPolygonStipple)(GLubyte * mask);
const GLubyte* (APIENTRY * drx_glGetString)(GLenum name);
void(APIENTRY * drx_glGetTexEnvfv)(GLenum target, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetTexEnviv)(GLenum target, GLenum pname, GLint * params);
void(APIENTRY * drx_glGetTexGendv)(GLenum coord, GLenum pname, GLdouble * params);
void(APIENTRY * drx_glGetTexGenfv)(GLenum coord, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetTexGeniv)(GLenum coord, GLenum pname, GLint * params);
void(APIENTRY * drx_glGetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels);
void(APIENTRY * drx_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint * params);
void(APIENTRY * drx_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat * params);
void(APIENTRY * drx_glGetTexParameteriv)(GLenum target, GLenum pname, GLint * params);
void(APIENTRY * drx_glHint)(GLenum target, GLenum mode);
void(APIENTRY * drx_glIndexMask)(GLuint mask);
void(APIENTRY * drx_glIndexPointer)(GLenum type, GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glIndexd)(GLdouble c);
void(APIENTRY * drx_glIndexdv)(const GLdouble * c);
void(APIENTRY * drx_glIndexf)(GLfloat c);
void(APIENTRY * drx_glIndexfv)(const GLfloat * c);
void(APIENTRY * drx_glIndexi)(GLint c);
void(APIENTRY * drx_glIndexiv)(const GLint * c);
void(APIENTRY * drx_glIndexs)(GLshort c);
void(APIENTRY * drx_glIndexsv)(const GLshort * c);
void(APIENTRY * drx_glIndexub)(GLubyte c);
void(APIENTRY * drx_glIndexubv)(const GLubyte * c);
void(APIENTRY * drx_glInitNames)(void);
void(APIENTRY * drx_glInterleavedArrays)(GLenum format, GLsizei stride, const GLvoid * pointer);
GLboolean(APIENTRY * drx_glIsEnabled)(GLenum cap);
GLboolean(APIENTRY * drx_glIsList)(GLuint list);
GLboolean(APIENTRY * drx_glIsTexture)(GLuint texture);
void(APIENTRY * drx_glLightModelf)(GLenum pname, GLfloat param);
void(APIENTRY * drx_glLightModelfv)(GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glLightModeli)(GLenum pname, GLint param);
void(APIENTRY * drx_glLightModeliv)(GLenum pname, const GLint * params);
void(APIENTRY * drx_glLightf)(GLenum light, GLenum pname, GLfloat param);
void(APIENTRY * drx_glLightfv)(GLenum light, GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glLighti)(GLenum light, GLenum pname, GLint param);
void(APIENTRY * drx_glLightiv)(GLenum light, GLenum pname, const GLint * params);
void(APIENTRY * drx_glLineStipple)(GLint factor, GLushort pattern);
void(APIENTRY * drx_glLineWidth)(GLfloat width);
void(APIENTRY * drx_glListBase)(GLuint base);
void(APIENTRY * drx_glLoadIdentity)(void);
void(APIENTRY * drx_glLoadMatrixd)(const GLdouble * m);
void(APIENTRY * drx_glLoadMatrixf)(const GLfloat * m);
void(APIENTRY * drx_glLoadName)(GLuint name);
void(APIENTRY * drx_glLogicOp)(GLenum opcode);
void(APIENTRY * drx_glMap1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points);
void(APIENTRY * drx_glMap1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points);
void(APIENTRY * drx_glMap2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points);
void(APIENTRY * drx_glMap2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points);
void(APIENTRY * drx_glMapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
void(APIENTRY * drx_glMapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
void(APIENTRY * drx_glMapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void(APIENTRY * drx_glMapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void(APIENTRY * drx_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
void(APIENTRY * drx_glMaterialfv)(GLenum face, GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glMateriali)(GLenum face, GLenum pname, GLint param);
void(APIENTRY * drx_glMaterialiv)(GLenum face, GLenum pname, const GLint * params);
void(APIENTRY * drx_glMatrixMode)(GLenum mode);
void(APIENTRY * drx_glMultMatrixd)(const GLdouble * m);
void(APIENTRY * drx_glMultMatrixf)(const GLfloat * m);
void(APIENTRY * drx_glNewList)(GLuint list, GLenum mode);
void(APIENTRY * drx_glNormal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
void(APIENTRY * drx_glNormal3bv)(const GLbyte * v);
void(APIENTRY * drx_glNormal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
void(APIENTRY * drx_glNormal3dv)(const GLdouble * v);
void(APIENTRY * drx_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
void(APIENTRY * drx_glNormal3fv)(const GLfloat * v);
void(APIENTRY * drx_glNormal3i)(GLint nx, GLint ny, GLint nz);
void(APIENTRY * drx_glNormal3iv)(const GLint * v);
void(APIENTRY * drx_glNormal3s)(GLshort nx, GLshort ny, GLshort nz);
void(APIENTRY * drx_glNormal3sv)(const GLshort * v);
void(APIENTRY * drx_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glOrtho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void(APIENTRY * drx_glPassThrough)(GLfloat token);
void(APIENTRY * drx_glPixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat * values);
void(APIENTRY * drx_glPixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint * values);
void(APIENTRY * drx_glPixelMapusv)(GLenum map, GLsizei mapsize, const GLushort * values);
void(APIENTRY * drx_glPixelStoref)(GLenum pname, GLfloat param);
void(APIENTRY * drx_glPixelStorei)(GLenum pname, GLint param);
void(APIENTRY * drx_glPixelTransferf)(GLenum pname, GLfloat param);
void(APIENTRY * drx_glPixelTransferi)(GLenum pname, GLint param);
void(APIENTRY * drx_glPixelZoom)(GLfloat xfactor, GLfloat yfactor);
void(APIENTRY * drx_glPointSize)(GLfloat size);
void(APIENTRY * drx_glPolygonMode)(GLenum face, GLenum mode);
void(APIENTRY * drx_glPolygonOffset)(GLfloat factor, GLfloat units);
void(APIENTRY * drx_glPolygonStipple)(const GLubyte * mask);
void(APIENTRY * drx_glPopAttrib)(void);
void(APIENTRY * drx_glPopClientAttrib)(void);
void(APIENTRY * drx_glPopMatrix)(void);
void(APIENTRY * drx_glPopName)(void);
void(APIENTRY * drx_glPrioritizeTextures)(GLsizei n, const GLuint * textures, const GLclampf * priorities);
void(APIENTRY * drx_glPushAttrib)(GLbitfield mask);
void(APIENTRY * drx_glPushClientAttrib)(GLbitfield mask);
void(APIENTRY * drx_glPushMatrix)(void);
void(APIENTRY * drx_glPushName)(GLuint name);
void(APIENTRY * drx_glRasterPos2d)(GLdouble x, GLdouble y);
void(APIENTRY * drx_glRasterPos2dv)(const GLdouble * v);
void(APIENTRY * drx_glRasterPos2f)(GLfloat x, GLfloat y);
void(APIENTRY * drx_glRasterPos2fv)(const GLfloat * v);
void(APIENTRY * drx_glRasterPos2i)(GLint x, GLint y);
void(APIENTRY * drx_glRasterPos2iv)(const GLint * v);
void(APIENTRY * drx_glRasterPos2s)(GLshort x, GLshort y);
void(APIENTRY * drx_glRasterPos2sv)(const GLshort * v);
void(APIENTRY * drx_glRasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
void(APIENTRY * drx_glRasterPos3dv)(const GLdouble * v);
void(APIENTRY * drx_glRasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
void(APIENTRY * drx_glRasterPos3fv)(const GLfloat * v);
void(APIENTRY * drx_glRasterPos3i)(GLint x, GLint y, GLint z);
void(APIENTRY * drx_glRasterPos3iv)(const GLint * v);
void(APIENTRY * drx_glRasterPos3s)(GLshort x, GLshort y, GLshort z);
void(APIENTRY * drx_glRasterPos3sv)(const GLshort * v);
void(APIENTRY * drx_glRasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void(APIENTRY * drx_glRasterPos4dv)(const GLdouble * v);
void(APIENTRY * drx_glRasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void(APIENTRY * drx_glRasterPos4fv)(const GLfloat * v);
void(APIENTRY * drx_glRasterPos4i)(GLint x, GLint y, GLint z, GLint w);
void(APIENTRY * drx_glRasterPos4iv)(const GLint * v);
void(APIENTRY * drx_glRasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
void(APIENTRY * drx_glRasterPos4sv)(const GLshort * v);
void(APIENTRY * drx_glReadBuffer)(GLenum mode);
void(APIENTRY * drx_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels);
void(APIENTRY * drx_glRectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void(APIENTRY * drx_glRectdv)(const GLdouble * v1, const GLdouble * v2);
void(APIENTRY * drx_glRectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void(APIENTRY * drx_glRectfv)(const GLfloat * v1, const GLfloat * v2);
void(APIENTRY * drx_glRecti)(GLint x1, GLint y1, GLint x2, GLint y2);
void(APIENTRY * drx_glRectiv)(const GLint * v1, const GLint * v2);
void(APIENTRY * drx_glRects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void(APIENTRY * drx_glRectsv)(const GLshort * v1, const GLshort * v2);
GLint(APIENTRY * drx_glRenderMode)(GLenum mode);
void(APIENTRY * drx_glRotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void(APIENTRY * drx_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void(APIENTRY * drx_glScaled)(GLdouble x, GLdouble y, GLdouble z);
void(APIENTRY * drx_glScalef)(GLfloat x, GLfloat y, GLfloat z);
void(APIENTRY * drx_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
void(APIENTRY * drx_glSelectBuffer)(GLsizei size, GLuint * buffer);
void(APIENTRY * drx_glShadeModel)(GLenum mode);
void(APIENTRY * drx_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
void(APIENTRY * drx_glStencilMask)(GLuint mask);
void(APIENTRY * drx_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
void(APIENTRY * drx_glTexCoord1d)(GLdouble s);
void(APIENTRY * drx_glTexCoord1dv)(const GLdouble * v);
void(APIENTRY * drx_glTexCoord1f)(GLfloat s);
void(APIENTRY * drx_glTexCoord1fv)(const GLfloat * v);
void(APIENTRY * drx_glTexCoord1i)(GLint s);
void(APIENTRY * drx_glTexCoord1iv)(const GLint * v);
void(APIENTRY * drx_glTexCoord1s)(GLshort s);
void(APIENTRY * drx_glTexCoord1sv)(const GLshort * v);
void(APIENTRY * drx_glTexCoord2d)(GLdouble s, GLdouble t);
void(APIENTRY * drx_glTexCoord2dv)(const GLdouble * v);
void(APIENTRY * drx_glTexCoord2f)(GLfloat s, GLfloat t);
void(APIENTRY * drx_glTexCoord2fv)(const GLfloat * v);
void(APIENTRY * drx_glTexCoord2i)(GLint s, GLint t);
void(APIENTRY * drx_glTexCoord2iv)(const GLint * v);
void(APIENTRY * drx_glTexCoord2s)(GLshort s, GLshort t);
void(APIENTRY * drx_glTexCoord2sv)(const GLshort * v);
void(APIENTRY * drx_glTexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
void(APIENTRY * drx_glTexCoord3dv)(const GLdouble * v);
void(APIENTRY * drx_glTexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
void(APIENTRY * drx_glTexCoord3fv)(const GLfloat * v);
void(APIENTRY * drx_glTexCoord3i)(GLint s, GLint t, GLint r);
void(APIENTRY * drx_glTexCoord3iv)(const GLint * v);
void(APIENTRY * drx_glTexCoord3s)(GLshort s, GLshort t, GLshort r);
void(APIENTRY * drx_glTexCoord3sv)(const GLshort * v);
void(APIENTRY * drx_glTexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void(APIENTRY * drx_glTexCoord4dv)(const GLdouble * v);
void(APIENTRY * drx_glTexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void(APIENTRY * drx_glTexCoord4fv)(const GLfloat * v);
void(APIENTRY * drx_glTexCoord4i)(GLint s, GLint t, GLint r, GLint q);
void(APIENTRY * drx_glTexCoord4iv)(const GLint * v);
void(APIENTRY * drx_glTexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
void(APIENTRY * drx_glTexCoord4sv)(const GLshort * v);
void(APIENTRY * drx_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
void(APIENTRY * drx_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glTexEnvi)(GLenum target, GLenum pname, GLint param);
void(APIENTRY * drx_glTexEnviv)(GLenum target, GLenum pname, const GLint * params);
void(APIENTRY * drx_glTexGend)(GLenum coord, GLenum pname, GLdouble param);
void(APIENTRY * drx_glTexGendv)(GLenum coord, GLenum pname, const GLdouble * params);
void(APIENTRY * drx_glTexGenf)(GLenum coord, GLenum pname, GLfloat param);
void(APIENTRY * drx_glTexGenfv)(GLenum coord, GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glTexGeni)(GLenum coord, GLenum pname, GLint param);
void(APIENTRY * drx_glTexGeniv)(GLenum coord, GLenum pname, const GLint * params);
void(APIENTRY * drx_glTexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
void(APIENTRY * drx_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels);
void(APIENTRY * drx_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
void(APIENTRY * drx_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat * params);
void(APIENTRY * drx_glTexParameteri)(GLenum target, GLenum pname, GLint param);
void(APIENTRY * drx_glTexParameteriv)(GLenum target, GLenum pname, const GLint * params);
void(APIENTRY * drx_glTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels);
void(APIENTRY * drx_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels);
void(APIENTRY * drx_glTranslated)(GLdouble x, GLdouble y, GLdouble z);
void(APIENTRY * drx_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
void(APIENTRY * drx_glVertex2d)(GLdouble x, GLdouble y);
void(APIENTRY * drx_glVertex2dv)(const GLdouble * v);
void(APIENTRY * drx_glVertex2f)(GLfloat x, GLfloat y);
void(APIENTRY * drx_glVertex2fv)(const GLfloat * v);
void(APIENTRY * drx_glVertex2i)(GLint x, GLint y);
void(APIENTRY * drx_glVertex2iv)(const GLint * v);
void(APIENTRY * drx_glVertex2s)(GLshort x, GLshort y);
void(APIENTRY * drx_glVertex2sv)(const GLshort * v);
void(APIENTRY * drx_glVertex3d)(GLdouble x, GLdouble y, GLdouble z);
void(APIENTRY * drx_glVertex3dv)(const GLdouble * v);
void(APIENTRY * drx_glVertex3f)(GLfloat x, GLfloat y, GLfloat z);
void(APIENTRY * drx_glVertex3fv)(const GLfloat * v);
void(APIENTRY * drx_glVertex3i)(GLint x, GLint y, GLint z);
void(APIENTRY * drx_glVertex3iv)(const GLint * v);
void(APIENTRY * drx_glVertex3s)(GLshort x, GLshort y, GLshort z);
void(APIENTRY * drx_glVertex3sv)(const GLshort * v);
void(APIENTRY * drx_glVertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void(APIENTRY * drx_glVertex4dv)(const GLdouble * v);
void(APIENTRY * drx_glVertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void(APIENTRY * drx_glVertex4fv)(const GLfloat * v);
void(APIENTRY * drx_glVertex4i)(GLint x, GLint y, GLint z, GLint w);
void(APIENTRY * drx_glVertex4iv)(const GLint * v);
void(APIENTRY * drx_glVertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
void(APIENTRY * drx_glVertex4sv)(const GLshort * v);
void(APIENTRY * drx_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);
void(APIENTRY * drx_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
};

using namespace drx_opengl;

	#define GET_OPENGL_FUNCTION(param)     *((INT_PTR*)&drx_ ## param) = (INT_PTR)DrxGetProcAddress(hOpenGL, # param)
	#define GET_OPENGL_GLU_FUNCTION(param) *((INT_PTR*)&drx_ ## param) = (INT_PTR)DrxGetProcAddress(hOpenGL_GLU, # param)

static bool LoadOpenGL()
{
	static HMODULE hOpenGL = DrxLoadLibrary((string("opengl32") + DrxSharedLibraryExtension).c_str());
	if (!hOpenGL)
		return false;

	#ifdef WIN32
	static HMODULE hOpenGL_GLU = GetModuleHandle("glu32.dll");
	if (!hOpenGL_GLU)
		return false;
	#endif //WIN32

	GET_OPENGL_FUNCTION(glDeleteLists);
	GET_OPENGL_FUNCTION(glGetIntegerv);
	GET_OPENGL_FUNCTION(glGetDoublev);
	GET_OPENGL_FUNCTION(glLoadIdentity);
	GET_OPENGL_FUNCTION(glMatrixMode);
	GET_OPENGL_FUNCTION(glViewport);
	GET_OPENGL_FUNCTION(glEnable);
	GET_OPENGL_FUNCTION(glPolygonMode);
	GET_OPENGL_FUNCTION(glShadeModel);
	GET_OPENGL_FUNCTION(glFlush);
	GET_OPENGL_FUNCTION(glCallLists);
	GET_OPENGL_FUNCTION(glRasterPos2f);
	GET_OPENGL_FUNCTION(glListBase);
	GET_OPENGL_FUNCTION(glPopMatrix);
	GET_OPENGL_FUNCTION(glTranslatef);
	GET_OPENGL_FUNCTION(glPushMatrix);
	GET_OPENGL_FUNCTION(glColor3fv);
	GET_OPENGL_FUNCTION(glDrawArrays);
	GET_OPENGL_FUNCTION(glInterleavedArrays);
	GET_OPENGL_FUNCTION(glColor3f);
	GET_OPENGL_FUNCTION(glClear);
	GET_OPENGL_FUNCTION(glClearColor);

	GET_OPENGL_FUNCTION(wglCreateContext);
	GET_OPENGL_FUNCTION(wglDeleteContext);
	GET_OPENGL_FUNCTION(wglMakeCurrent);
	//GET_OPENGL_FUNCTION( wglUseFontBitmaps );
	// Use non Unicode version
	*((INT_PTR*)&drx_wglUseFontBitmaps) = (INT_PTR)GetProcAddress(hOpenGL, "wglUseFontBitmapsA");

	GET_OPENGL_GLU_FUNCTION(gluPerspective);
	GET_OPENGL_GLU_FUNCTION(gluLookAt);
	GET_OPENGL_GLU_FUNCTION(gluProject);
	GET_OPENGL_GLU_FUNCTION(gluSphere);
	GET_OPENGL_GLU_FUNCTION(gluNewQuadric);
	GET_OPENGL_GLU_FUNCTION(gluDeleteQuadric);

	return true;
}

bool CNULLRenderAuxGeom::EnableOpenGL()
{
	if (!LoadOpenGL())
	{
		return false;
	}

	CCamera camera = gEnv->pSystem->GetViewCamera();
	camera.SetFrustum(static_cast<i32>(W), static_cast<i32>(H));

	const float FOV = camera.GetFov() / PI * 180.0f;
	const float PNR = camera.GetNearPlane();
	const float PFR = camera.GetFarPlane();

	gEnv->pSystem->SetViewCamera(camera);

	PIXELFORMATDESCRIPTOR pfd;
	i32 format;

	// get the device context (DC)
	m_hdc = GetDC(m_hwnd);

	// set the pixel format for the DC
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(m_hdc, &pfd);
	SetPixelFormat(m_hdc, format, &pfd);

	// create and enable the render context (RC)
	m_glrc = drx_wglCreateContext(m_hdc);
	drx_wglMakeCurrent(m_hdc, m_glrc);

	drx_glShadeModel(GL_FLAT);
	drx_glPolygonMode(GL_FRONT, GL_FILL);
	drx_glEnable(GL_DEPTH_TEST);

	drx_glViewport(0, 0, static_cast<GLsizei>(W), static_cast<GLsizei>(H));
	drx_glMatrixMode(GL_PROJECTION);
	drx_glLoadIdentity();
	drx_gluPerspective(FOV, W / H, PNR, PFR);
	drx_glMatrixMode(GL_MODELVIEW);
	drx_glLoadIdentity();

	m_qobj = drx_gluNewQuadric();

	return true;
}

void CNULLRenderAuxGeom::DisableOpenGL()
{
	if (m_glLoaded)
	{
		drx_gluDeleteQuadric(m_qobj);
		drx_wglMakeCurrent(NULL, NULL);
		drx_wglDeleteContext(m_glrc);
	}
	ReleaseDC(m_hwnd, m_hdc);
}

#endif

CNULLRenderAuxGeom::CNULLRenderAuxGeom()
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	tukk wndClassName = "DebugRenderer";

	// register window class
	WNDCLASS wc;
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = DrxGetCurrentModule();
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = wndClassName;
	RegisterClass(&wc);

	// create main window
	m_hwnd = CreateWindow(
	  wndClassName, wndClassName,
	  WS_CAPTION | WS_POPUP,
	  0, 0, static_cast<i32>(W), static_cast<i32>(H),
	  NULL, NULL, wc.hInstance, NULL);

	ShowWindow(m_hwnd, SW_HIDE);
	UpdateWindow(m_hwnd);

	m_glLoaded = EnableOpenGL();
	if (m_glLoaded)
	{
		m_eye.Set(0.0f, 0.0f, 0.0f);
		m_dir.Set(0.0f, 1.0f, 0.0f);
		m_up.Set(0.0f, 0.0f, 1.0f);
		m_updateSystemView = true;
	}

	// Register the commands regardless of gl initialization to let the player
	// use them and see the warning.
	REGISTER_COMMAND("r_debug_renderer_show_window", DebugRendererShowWindow, VF_NULL, "");
	REGISTER_COMMAND("r_debug_renderer_set_eye_pos", DebugRendererSetEyePos, VF_NULL, "");
	REGISTER_COMMAND("r_debug_renderer_update_system_view", DebugRendererUpdateSystemView, VF_NULL, "");
#endif
}

CNULLRenderAuxGeom::~CNULLRenderAuxGeom()
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	DisableOpenGL();
	DestroyWindow(m_hwnd);
#endif
}

void CNULLRenderAuxGeom::BeginFrame()
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);

	gEnv->pSystem->PumpWindowMessage(false);

	{
		m_dir.normalize();
		m_up.normalize();

		Vec3 right = m_dir ^ m_up;

		if (s_active)
		{
			Matrix34 m;

			m.SetIdentity();
			if (GetAsyncKeyState('W') & 0x8000)
				m.AddTranslation(m_dir);
			if (GetAsyncKeyState('S') & 0x8000)
				m.AddTranslation(-m_dir);
			if (GetAsyncKeyState('A') & 0x8000)
				m.AddTranslation(-right);
			if (GetAsyncKeyState('D') & 0x8000)
				m.AddTranslation(right);
			m_eye = m * m_eye;

			m.SetIdentity();
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
				m.SetRotationAA(-PI / 180.0f * THETA, VUP); // !m_up
			if (GetAsyncKeyState(VK_LEFT) & 0x8000)
				m.SetRotationAA(PI / 180.0f * THETA, VUP); // !m_up
			if (GetAsyncKeyState(VK_UP) & 0x8000)
				m.SetRotationAA(PI / 180.0f * THETA, right);
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)
				m.SetRotationAA(-PI / 180.0f * THETA, right);
			m_up = m * m_up;
			m_dir = m * m_dir;
		}

		if (m_updateSystemView)
		{
			Matrix34 m(Matrix33::CreateOrientation(m_dir, m_up, 0), m_eye);
			CCamera cam = gEnv->pSystem->GetViewCamera();
			cam.SetMatrix(m);
			gEnv->pSystem->SetViewCamera(cam);
		}
		else
		{
			const Matrix34& viewMatrix = gEnv->pSystem->GetViewCamera().GetMatrix();
			m_eye = viewMatrix.GetTranslation();
			m_dir = viewMatrix.GetColumn1();
			m_up = viewMatrix.GetColumn2();
		}
	}

#endif
}

void CNULLRenderAuxGeom::EndFrame()
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);

	if (!s_hidden)
	{
		drx_glLoadIdentity();

		Vec3 at = m_eye + m_dir;
		drx_gluLookAt(m_eye.x, m_eye.y, m_eye.z, at.x, at.y, at.z, m_up.x, m_up.y, m_up.z);

		drx_glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
		drx_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drx_glColor3f(1.0f, 0.0f, 0.0f);
		drx_gluSphere(m_qobj, 1.0f, 32, 32);

		drx_glInterleavedArrays(GL_C3F_V3F, 0, &m_points[0]);
		drx_glDrawArrays(GL_POINTS, 0, m_points.size());

		drx_glInterleavedArrays(GL_C3F_V3F, 0, &m_lines[0]);
		drx_glDrawArrays(GL_LINES, 0, m_lines.size() * 2);

		for (size_t i = 0; i < m_polyLines.size(); ++i)
		{
			const SPolyLine& polyline = m_polyLines[i];
			drx_glInterleavedArrays(GL_C3F_V3F, 0, &polyline.points[0]);
			drx_glDrawArrays(GL_LINE_STRIP, 0, polyline.points.size());
		}

		drx_glInterleavedArrays(GL_C3F_V3F, 0, &m_triangles[0]);
		drx_glDrawArrays(GL_TRIANGLES, 0, m_triangles.size() * 3);

		for (size_t i = 0; i < m_spheres.size(); ++i)
		{
			drx_glColor3fv(m_spheres[i].p.color);
			drx_glPushMatrix();
			drx_glLoadIdentity();
			drx_glTranslatef(m_spheres[i].p.vertex[0], m_spheres[i].p.vertex[1], m_spheres[i].p.vertex[2]);
			drx_gluSphere(m_qobj, m_spheres[i].r, 32, 32);
			drx_glPopMatrix();
		}

		drx_glFlush();

		SwapBuffers(m_hdc);
	}

	m_points.resize(0);
	m_lines.resize(0);
	m_polyLines.resize(0);
	m_triangles.resize(0);
	m_spheres.resize(0);
#endif
}

void CNULLRenderAuxGeom::DrawPoint(const Vec3& v, const ColorB& col, u8 size /* = 1  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	m_points.push_back(SPoint(v, col));
#endif
}

void CNULLRenderAuxGeom::DrawPoints(const Vec3* v, u32 numPoints, const ColorB* col, u8 size /* = 1  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	for (u32 i = 0; i < numPoints; ++i)
		m_points.push_back(SPoint(v[i], *col));
#endif
}

void CNULLRenderAuxGeom::DrawPoints(const Vec3* v, u32 numPoints, const ColorB& col, u8 size /* = 1  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	for (u32 i = 0; i < numPoints; ++i)
		m_points.push_back(SPoint(v[i], col));
#endif
}

void CNULLRenderAuxGeom::DrawLine(const Vec3& v0, const ColorB& colV0, const Vec3& v1, const ColorB& colV1, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	m_lines.push_back(SLine(SPoint(v0, colV0), SPoint(v1, colV1)));
#endif
}

void CNULLRenderAuxGeom::DrawLines(const Vec3* v, u32 numPoints, const ColorB& col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert((numPoints >= 2) && !(numPoints & 1));
	for (u32 i = 0; i < numPoints; i += 2)
		m_lines.push_back(SLine(SPoint(v[i], col), SPoint(v[i + 1], col)));
#endif
}

void CNULLRenderAuxGeom::DrawLines(const Vec3* v, u32 numPoints, const ColorB* col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert((numPoints >= 2) && !(numPoints & 1));
	for (u32 i = 0; i < numPoints; i += 2)
		m_lines.push_back(SLine(SPoint(v[i], *col), SPoint(v[i + 1], *col)));
#endif
}

void CNULLRenderAuxGeom::DrawLines(const Vec3* v, u32k* packedColorARGB8888, u32 numPoints, float thickness, bool alphaFlag)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert((numPoints >= 2) && !(numPoints & 1));
	for (u32 i = 0; i < numPoints; i += 2)
		m_lines.push_back(SLine(SPoint(v[i], packedColorARGB8888[i]), SPoint(v[i + 1], packedColorARGB8888[i + 1])));
#endif
}

void CNULLRenderAuxGeom::DrawLines(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB& col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 2);
	assert((numIndices >= 2) && !(numIndices & 1));
	for (u32 i = 0; i < numIndices; i += 2)
	{
		vtx_idx i0 = ind[i], i1 = ind[i + 1];
		assert((i0 < numPoints) && (i1 < numPoints));
		m_lines.push_back(SLine(SPoint(v[i0], col), SPoint(v[i1], col)));
	}
#endif
}

void CNULLRenderAuxGeom::DrawLineStrip(const Vec3* v, u32 numPoints, const ColorB* col, float thickness /* = 1.0f*/)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(false && "ToDo: Not implemented.");
#endif
}

void CNULLRenderAuxGeom::DrawLines(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB* col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 2);
	assert((numIndices >= 2) && !(numIndices & 1));
	for (u32 i = 0; i < numIndices; i += 2)
	{
		vtx_idx i0 = ind[i], i1 = ind[i + 1];
		assert((i0 < numPoints) && (i1 < numPoints));
		m_lines.push_back(SLine(SPoint(v[i0], *col), SPoint(v[i1], *col)));
	}
#endif
}

void CNULLRenderAuxGeom::DrawPolyline(const Vec3* v, u32 numPoints, bool closed, const ColorB& col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 2);
	assert(!closed || (numPoints >= 3));   // if "closed" then we need at least three vertices
	m_polyLines.resize(m_polyLines.size() + 1);
	SPolyLine& polyline = m_polyLines[m_polyLines.size() - 1];
	for (u32 i = 0; i < numPoints; ++i)
		polyline.points.push_back(SPoint(v[i], col));
	if (closed)
		polyline.points.push_back(SPoint(v[0], col));
#endif
}

void CNULLRenderAuxGeom::DrawPolyline(const Vec3* v, u32 numPoints, bool closed, const ColorB* col, float thickness /* = 1::0f  */)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 2);
	assert(!closed || (numPoints >= 3));   // if "closed" then we need at least three vertices
	m_polyLines.resize(m_polyLines.size() + 1);
	SPolyLine& polyline = m_polyLines[m_polyLines.size() - 1];
	for (u32 i = 0; i < numPoints; ++i)
		polyline.points.push_back(SPoint(v[i], *col));
	if (closed)
		polyline.points.push_back(SPoint(v[0], *col));
#endif
}

void CNULLRenderAuxGeom::DrawTriangle(const Vec3& v0, const ColorB& colV0, const Vec3& v1, const ColorB& colV1, const Vec3& v2, const ColorB& colV2)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	m_triangles.push_back(STriangle(SPoint(v0, colV0), SPoint(v1, colV1), SPoint(v2, colV2)));
#endif
}

void CNULLRenderAuxGeom::DrawTriangles(const Vec3* v, u32 numPoints, const ColorB& col)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert((numPoints >= 3) && !(numPoints % 3));
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	for (size_t i = 0; i < numPoints; i += 3)
		m_triangles.push_back(STriangle(SPoint(v[i], col), SPoint(v[i + 1], col), SPoint(v[i + 2], col)));
#endif
}

void CNULLRenderAuxGeom::DrawTriangles(const Vec3* v, u32 numPoints, const ColorB* col)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert((numPoints >= 3) && !(numPoints % 3));
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	for (size_t i = 0; i < numPoints; i += 3)
		m_triangles.push_back(STriangle(SPoint(v[i], *col), SPoint(v[i + 1], *col), SPoint(v[i + 2], *col)));
#endif
}

void CNULLRenderAuxGeom::DrawTriangles(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB& col)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 3);
	assert((numIndices >= 3) && !(numIndices % 3));
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	for (size_t i = 0; i < numIndices; i += 3)
	{
		vtx_idx i0 = ind[i], i1 = ind[i + 1], i2 = ind[i + 2];
		assert((i0 < numPoints) && (i1 < numPoints) && (i2 < numPoints));
		m_triangles.push_back(STriangle(SPoint(v[i0], col), SPoint(v[i1], col), SPoint(v[i2], col)));
	}
#endif
}

void CNULLRenderAuxGeom::DrawTriangles(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB* col)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	assert(numPoints >= 3);
	assert((numIndices >= 3) && !(numIndices % 3));
	DRX_PROFILE_FUNCTION(PROFILE_RENDERER);
	for (size_t i = 0; i < numIndices; i += 3)
	{
		vtx_idx i0 = ind[i], i1 = ind[i + 1], i2 = ind[i + 2];
		assert((i0 < numPoints) && (i1 < numPoints) && (i2 < numPoints));
		m_triangles.push_back(STriangle(SPoint(v[i0], *col), SPoint(v[i1], *col), SPoint(v[i2], *col)));
	}
#endif
}

void CNULLRenderAuxGeom::DrawSphere(const Vec3& pos, float radius, const ColorB& col, bool drawShaded)
{
#ifdef ENABLE_WGL_DEBUG_RENDERER
	m_spheres.push_back(SSphere(SPoint(pos, col), radius));
#endif
}

void CNULLRenderAuxGeom::PushImage(const SRender2DImageDescription &image)
{

}