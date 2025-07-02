// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "IEditor.h" // for AxisConstrains and RefCoordSys
#include "Gizmo.h"

struct DisplayContext;
struct HitContext;
struct IDisplayViewport;

//////////////////////////////////////////////////////////////////////////
// CUniformScaleGizmo Gizmo. 
// 
// Allows view space movement or rotation around its center or trackball style rotation
//////////////////////////////////////////////////////////////////////////
class EDITOR_COMMON_API CUniformScaleGizmo : public CGizmo
{
public:
	CUniformScaleGizmo();
	~CUniformScaleGizmo();

	//! set position - should be world space
	void SetPosition(Vec3 pos);
	//! set rgb color of the gizmo
	void SetColor(Vec3 color);
	//! set unique scale of the gizmo
	void SetScale(float scale);
	//! set x vector of the plane - should be world space
	void SetXVector(Vec3 dir);
	//! set y vector of the plane - should be world space
	void SetYVector(Vec3 dir);

	virtual void Display(DisplayContext& dc) override;

	virtual bool MouseCallback(IDisplayViewport* view, EMouseEvent event, CPoint& point, i32 nFlags) override;

	virtual void GetWorldBounds(AABB& bbox) override;

	virtual bool HitTest(HitContext& hc) override;

	// emitted when user starts dragging the gizmo
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, const CPoint& point, i32 nFlags)> signalBeginDrag;

	// emitted while dragging.
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, float scale, const CPoint& point, i32 nFlags)> signalDragging;

	// emitted when finished dragging
	CDrxSignal <void(IDisplayViewport* view, CGizmo* gizmo, const CPoint& point, i32 nFlags)> signalEndDrag;
private:
	// position and vectors defining the plane in world space
	Vec3 m_position;
	Vec3 m_color;
	Vec3 m_xAxis;
	Vec3 m_yAxis;

	//! custom scale of widget - final size is calculated from z-distance of widget to camera and global parameters
	float m_scale;

	CPoint m_initPoint;
	float m_interactionScale;
};


