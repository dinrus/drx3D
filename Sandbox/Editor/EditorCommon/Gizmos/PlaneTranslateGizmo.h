// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IEditor.h" // for AxisConstrains and RefCoordSys
#include "Gizmo.h"

struct DisplayContext;
struct HitContext;
struct IDisplayViewport;

//////////////////////////////////////////////////////////////////////////
// CPlaneTranslateGizmo Gizmo. 
// 
// Allows constrained movement on the plane of the gizmo
//////////////////////////////////////////////////////////////////////////
class EDITOR_COMMON_API CPlaneTranslateGizmo : public CGizmo
{
public:
	CPlaneTranslateGizmo();
	~CPlaneTranslateGizmo();

	//! set position - should be world space
	void SetPosition(Vec3 pos);
	//! set x vector of the plane - should be world space
	void SetXVector(Vec3 dir);
	//! set y vector of the plane - should be world space
	void SetYVector(Vec3 dir);
	//! set rgb color of the gizmo
	void SetColor(Vec3 color);
	//! set unique scale of the gizmo
	void SetScale(float scale);
	//! set x offset from position along the length of the x vector
	void SetXOffset(float offset);
	//! set y offset from position along the length of the x vector
	void SetYOffset(float offset);

	virtual void Display(DisplayContext& dc) override;

	virtual bool MouseCallback(IDisplayViewport* view, EMouseEvent event, CPoint& point, i32 nFlags) override;

	virtual void GetWorldBounds(AABB& bbox) override;

	virtual bool HitTest(HitContext& hc) override;

	// emitted when user starts dragging the gizmo
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, const Vec3& initialPosition, const CPoint& point, i32 nFlags)> signalBeginDrag;

	// emitted while dragging.
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, const Vec3& offset, const CPoint& point, i32 nFlags)> signalDragging;

	// emitted when finished dragging
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, const CPoint& point, i32 nFlags)> signalEndDrag;

private:
	void DrawPlane(DisplayContext& dc, Vec3 position);

	// position and vectors defining the plane in world space
	Vec3 m_position;
	Vec3 m_xVector;
	Vec3 m_yVector;
	Vec3 m_color;

	//! custom scale of widget - final size is calculated from z-distance of widget to camera and global parameters
	float m_scale;
	float m_xOffset;
	float m_yOffset;

	//interaction data;
	Vec3 m_initPosition;
	Vec3 m_initOffset;
	Vec3 m_interactionOffset;
};


