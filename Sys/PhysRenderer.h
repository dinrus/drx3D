// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File: PhysRenderer.h
//  Описание: declaration of a simple dedicated renderer for the physics subsystem
//
//	История:
//
//////////////////////////////////////////////////////////////////////

#ifndef PHYSRENDERER_H
#define PHYSRENDERER_H

#include <drx3D/Phys/IPhysicsDebugRenderer.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

#if _MSC_VER > 1000
	#pragma once
#endif

struct SRayRec
{
	Vec3  origin;
	Vec3  dir;
	float time;
	i32   idxColor;
};

struct SGeomRec
{
	i32      itype;
	char     buf[sizeof(primitives::box)];
	Vec3     offset;
	Matrix33 R;
	float    scale;
	Vec3     sweepDir;
	float    time;
};

class CPhysRenderer : public IPhysicsDebugRenderer, public IPhysRenderer
{
public:
	CPhysRenderer();
	~CPhysRenderer();
	void Init();
	void DrawGeometry(IGeometry* pGeom, geom_world_data* pgwd, const ColorB& clr, const Vec3& sweepDir = Vec3(0));
	void DrawGeometry(i32 itype, ukk pGeomData, geom_world_data* pgwd, const ColorB& clr, const Vec3& sweepDir = Vec3(0));
	QuatT SetOffset(const Vec3& offs = Vec3(ZERO), const Quat& qrot = Quat(ZERO)) 
	{ 
		QuatT prev(m_qrot,m_offset); 
		m_offset = offs; 
		if ((qrot|qrot)>0)
			m_qrot = qrot;
		return prev; 
	}

	// IPhysRenderer
	virtual void DrawFrame(const Vec3& pnt, const Vec3* axes, const float scale, const Vec3* limits, i32k axes_locked);
	virtual void DrawGeometry(IGeometry* pGeom, geom_world_data* pgwd, i32 idxColor = 0, i32 bSlowFadein = 0, const Vec3& sweepDir = Vec3(0), const ColorF& color = ColorF(1, 1, 1, 1));
	virtual void DrawLine(const Vec3& pt0, const Vec3& pt1, i32 idxColor = 0, i32 bSlowFadein = 0);
	virtual void DrawText(const Vec3& pt, tukk txt, i32 idxColor, float saturation = 0)
	{
		if (!m_camera.IsPointVisible(pt))
			return;
		float clr[4] = { min(saturation * 2, 1.0f), 0, 0, 1 };
		clr[1] = min((1 - saturation) * 2, 1.0f) * 0.5f;
		clr[idxColor + 1] = min((1 - saturation) * 2, 1.0f);
		IRenderAuxText::DrawLabelEx(pt, 1.5f, clr, true, true, txt);
	}
	static tukk  GetPhysForeignName(uk pForeignData, i32 iForeignData, i32 iForeignFlags);
	virtual tukk GetForeignName(uk pForeignData, i32 iForeignData, i32 iForeignFlags)
	{ return GetPhysForeignName(pForeignData, iForeignData, iForeignFlags); }
	// ^^^

	// IPhysicsDebugRenderer
	virtual void UpdateCamera(const CCamera& camera);
	virtual void DrawAllHelpers(IPhysicalWorld* world);
	virtual void DrawEntityHelpers(IPhysicalWorld* world, i32 entityId, i32 iDrawHelpers);
	virtual void DrawEntityHelpers(IPhysicalEntity* entity, i32 iDrawHelpers);
	virtual void Flush(float dt);
	// ^^^

	float m_cullDist, m_wireframeDist;
	float m_timeRayFadein;
	float m_rayPeakTime;
	i32   m_maxTris, m_maxTrisRange;

protected:
	CCamera          m_camera;
	IRenderer*       m_pRenderer;
	SRayRec*         m_rayBuf;
	i32              m_szRayBuf, m_iFirstRay, m_iLastRay, m_nRays;
	SGeomRec*        m_geomBuf;
	i32              m_szGeomBuf, m_iFirstGeom, m_iLastGeom, m_nGeoms;
	IGeometry*       m_pRayGeom;
	primitives::ray* m_pRay;
	Vec3             m_offset;
	Quat             m_qrot;
	static ColorB    g_colorTab[9];
	 i32     m_lockDrawGeometry;
};

#endif
