// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RULER_H__
#define __RULER_H__

#include "RulerPoint.h"

#include <EditorFramework/Preferences.h>
#include <drx3D/CoreX/Serialization/yasli/decorators/Range.h>

// Preferences
struct SRulerPreferences : public SPreferencePage
{
	SRulerPreferences()
		: SPreferencePage("Ruler", "Viewport/Gizmo")
		, rulerSphereScale(0.25f)
		, rulerSphereTrans(0.5f)
	{
	}

	virtual bool Serialize(yasli::Archive& ar) override
	{
		ar.openBlock("Ruler", "Ruler");
		ar(yasli::Range(rulerSphereScale, 0.01f, 100.f), "rulerSphereScale", "Ruler Sphere Scale");
		ar(yasli::Range(rulerSphereTrans, 0.01f, 100.f), "rulerSphereTrans", "Ruler Sphere Transparency");
		ar.closeBlock();

		return true;
	}

	// Scale size and transparency for debug spheres when using Ruler tool
	float rulerSphereScale;
	float rulerSphereTrans;
};

//! The Ruler utility helps to determine distances between user-specified points.
class CRuler
{
public:
	CRuler();
	~CRuler();

	//! Activate the ruler
	void SetActive(bool bActive);
	bool IsActive() const { return m_bActive; }

	//! Update
	void Update();

	//! Mouse callback handling from viewport
	bool MouseCallback(CViewport* pView, EMouseEvent event, CPoint& point, i32 flags);

private:
	//! Mouse callback helpers
	void OnMouseMove(CViewport* pView, CPoint& point, i32 flags);
	void OnLButtonUp(CViewport* pView, CPoint& point, i32 flags);
	void OnMButtonUp(CViewport* pView, CPoint& point, i32 flags);
	void OnRButtonUp(CViewport* pView, CPoint& point, i32 flags);

	//! Returns world point based on mouse point
	void UpdateRulerPoint(CViewport* pView, const CPoint& point, CRulerPoint& rulerPoint, bool bRequestPath);

	bool IsObjectSelectMode() const;

private:
	bool    m_bActive;
	DrxGUID m_MouseOverObject;

	// Base point
	CRulerPoint m_startPoint;
	CRulerPoint m_endPoint;

	float       m_sphereScale;
	float       m_sphereTrans;
};

#endif //__RULER_H__

