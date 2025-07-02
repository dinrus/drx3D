// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef NULL_RENDER_AUX_GEOM_H
#define NULL_RENDER_AUX_GEOM_H

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

class CNULLRenderer;
class IDrxSizer;

#if DRX_PLATFORM_WINDOWS && !defined(_RELEASE)
	#define ENABLE_WGL_DEBUG_RENDERER
#endif

#ifdef ENABLE_WGL_DEBUG_RENDERER
	#include <drx3D/CoreX/Platform/DrxWindows.h>
	#include <gl/GL.h>
	#include <gl/GLU.h>
#endif

class CNULLRenderAuxGeom : public IRenderAuxGeom
{
public:
	// interface
	SAuxGeomRenderFlags SetRenderFlags(const SAuxGeomRenderFlags& renderFlags) final                                                                         { return SAuxGeomRenderFlags(); }
	SAuxGeomRenderFlags GetRenderFlags() final                                                                                                               { return SAuxGeomRenderFlags(); }
	
	const CCamera&      GetCamera() const final                                                                                                              { static CCamera camera; return camera; }

	//! Set current display context for the following auxiliary rendering.
	void                SetCurrentDisplayContext(const SDisplayContextKey& displayContextKey) final                                               {}

	void                DrawPoint(const Vec3& v, const ColorB& col, u8 size = 1) final;
	void                DrawPoints(const Vec3* v, u32 numPoints, const ColorB& col, u8 size = 1) final;
	void                DrawPoints(const Vec3* v, u32 numPoints, const ColorB* col, u8 size = 1) final;

	void                DrawLine(const Vec3& v0, const ColorB& colV0, const Vec3& v1, const ColorB& colV1, float thickness = 1.0f) final;
	void                DrawLines(const Vec3* v, u32 numPoints, const ColorB& col, float thickness = 1.0f) final;
	void                DrawLines(const Vec3* v, u32 numPoints, const ColorB* col, float thickness = 1.0f) final;
	void                DrawLines(const Vec3* v, u32k* packedColorARGB8888, u32 numPoints, float thickness = 1.0f, bool alphaFlag = true) final;
	void                DrawLines(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB& col, float thickness = 1.0f) final;
	void                DrawLines(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB* col, float thickness = 1.0f) final;
	void                DrawLineStrip(const Vec3* v, u32 numPoints, const ColorB* col, float thickness = 1.0f) final;
	void                DrawPolyline(const Vec3* v, u32 numPoints, bool closed, const ColorB& col, float thickness = 1.0f) final;
	void                DrawPolyline(const Vec3* v, u32 numPoints, bool closed, const ColorB* col, float thickness = 1.0f) final;

	void                DrawTriangle(const Vec3& v0, const ColorB& colV0, const Vec3& v1, const ColorB& colV1, const Vec3& v2, const ColorB& colV2) final;
	void                DrawTriangles(const Vec3* v, u32 numPoints, const ColorB& col) final;
	void                DrawTriangles(const Vec3* v, u32 numPoints, const ColorB* col) final;
	void                DrawTriangles(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB& col) final;
	void                DrawTriangles(const Vec3* v, u32 numPoints, const vtx_idx* ind, u32 numIndices, const ColorB* col) final;

	void                DrawBuffer(const SAuxVertex* inVertices, u32 numVertices, bool textured) final                                                    {}
	SAuxVertex*         BeginDrawBuffer(u32 maxVertices, bool textured) final                                                                             { return nullptr; }
	void                EndDrawBuffer(u32 numVertices) final                                                                                              {}

	void                DrawAABB(const AABB& aabb, bool bSolid, const ColorB& col, const EBoundingBoxDrawStyle& bbDrawStyle) final                           {}
	void                DrawAABBs(const AABB* aabb, u32 aabbCount, bool bSolid, const ColorB& col, const EBoundingBoxDrawStyle& bbDrawStyle) final        {}
	void                DrawAABB(const AABB& aabb, const Matrix34& matWorld, bool bSolid, const ColorB& col, const EBoundingBoxDrawStyle& bbDrawStyle) final {}

	void                DrawOBB(const OBB& obb, const Vec3& pos, bool bSolid, const ColorB& col, const EBoundingBoxDrawStyle& bbDrawStyle) final             {}
	void                DrawOBB(const OBB& obb, const Matrix34& matWorld, bool bSolid, const ColorB& col, const EBoundingBoxDrawStyle& bbDrawStyle) final    {}

	void                DrawSphere(const Vec3& pos, float radius, const ColorB& col, bool drawShaded = true) final;
	void                DrawCone(const Vec3& pos, const Vec3& dir, float radius, float height, const ColorB& col, bool drawShaded = true) final              {}
	void                DrawCylinder(const Vec3& pos, const Vec3& dir, float radius, float height, const ColorB& col, bool drawShaded = true) final          {}

	void                DrawBone(const Vec3& rParent, const Vec3& rBone, ColorB col) final                                                                   {}

	void                RenderTextQueued(Vec3 pos, const SDrawTextInfo& ti, tukk text) final                                                          {}

	void                PushImage(const SRender2DImageDescription &image) final;

	i32               PushMatrix(const Matrix34& mat) final                                                                                                { return -1; }
	Matrix34*           GetMatrix() final                                                                                                                    { return nullptr; }
	void                SetMatrixIndex(i32 matID) final                                                                                                    {}
	void                SetOrthographicProjection(bool enable, float l = 0, float r = 1, float b = 0, float t = 1, float n = -1e10, float f = 1e10) final    {}

	void                Submit(uint frames = 0) final                                                                                                        {}

public:
	static CNULLRenderAuxGeom* Create()
	{
		if (s_pThis == NULL)
			s_pThis = new CNULLRenderAuxGeom();
		return s_pThis;
	}

public:
	~CNULLRenderAuxGeom();

	void BeginFrame();
	void EndFrame();

private:
	CNULLRenderAuxGeom();

	static CNULLRenderAuxGeom* s_pThis;

#ifdef ENABLE_WGL_DEBUG_RENDERER
	static void DebugRendererShowWindow(IConsoleCmdArgs* args)
	{
		if (!s_pThis || !s_pThis->IsGLLoaded())
			return;

		ShowWindow(s_pThis->m_hwnd, s_hidden ? SW_SHOWNA : SW_HIDE);
		s_hidden = !s_hidden;
	}

	static void DebugRendererSetEyePos(IConsoleCmdArgs* args)
	{
		if (!s_pThis || !s_pThis->IsGLLoaded())
			return;

		if (args->GetArgCount() != 4)
			return;

		Vec3 pos;
		pos.x = (float)atof(args->GetArg(1));
		pos.y = (float)atof(args->GetArg(2));
		pos.z = (float)atof(args->GetArg(3));

		s_pThis->m_eye = pos;
	}

	static void DebugRendererUpdateSystemView(IConsoleCmdArgs* args)
	{
		if (!s_pThis || !s_pThis->IsGLLoaded())
			return;

		if (args->GetArgCount() != 2)
			return;

		s_pThis->m_updateSystemView = ((i32)atoi(args->GetArg(1)) != 0);
	}

	HWND  m_hwnd;
	HDC   m_hdc;
	HGLRC m_glrc;

	bool  m_glLoaded;
	bool IsGLLoaded() const
	{
		if (!m_glLoaded)
		{
			DrxLogAlways("Couldn't initialize OpenGL library, the debug renderer is unavailable");
		}
		return m_glLoaded;
	}

	GLUquadricObj* m_qobj;

	// Window Procedure
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_CREATE:
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		case WM_DESTROY:
			return 0;

		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
				s_active = true;
			else
				s_active = false;
			return 0;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	bool EnableOpenGL();
	void DisableOpenGL();

	struct SPoint // GL_C3F_V3F
	{
		GLfloat color[3];  // rgb
		GLfloat vertex[3]; // xyz

		SPoint() {}

		SPoint(const Vec3& v, const ColorB& c)
		{
			color[0] = c.r / 255.f;
			color[1] = c.g / 255.f;
			color[2] = c.b / 255.f;
			vertex[0] = v.x;
			vertex[1] = v.y;
			vertex[2] = v.z;
		}

		const SPoint& operator=(const SPoint& rhs)
		{
			memcpy(color, rhs.color, sizeof(color));
			memcpy(vertex, rhs.vertex, sizeof(vertex));
			return *this;
		}
	};

	std::vector<SPoint> m_points;

	struct SLine
	{
		SPoint points[2];

		SLine() {}
		SLine(const SPoint& p0, const SPoint& p1)
		{
			points[0] = p0;
			points[1] = p1;
		}
	};

	std::vector<SLine> m_lines;

	struct SPolyLine
	{
		std::vector<SPoint> points;
	};

	std::vector<SPolyLine> m_polyLines;

	struct STriangle
	{
		SPoint points[3];

		STriangle() {}
		STriangle(const SPoint& p0, const SPoint& p1, const SPoint& p2)
		{
			points[0] = p0;
			points[1] = p1;
			points[2] = p2;
		}
	};

	std::vector<STriangle> m_triangles;

	struct SSphere
	{
		SPoint p;
		float  r;

		SSphere() {}
		SSphere(const SPoint& p_, float r_) : p(p_), r(r_) {}
	};

	std::vector<SSphere> m_spheres;

	Vec3                 m_eye, m_dir, m_up;
	bool                 m_updateSystemView;

	static bool          s_active;
	static bool          s_hidden;
#endif
};

#endif // NULL_RENDER_AUX_GEOM_H
