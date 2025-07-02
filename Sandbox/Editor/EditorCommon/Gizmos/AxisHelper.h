// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __AxisHelper_h__
#define __AxisHelper_h__
#pragma once

#include "GizmoManager.h" // for AxisConstrains and RefCoordSys

struct DisplayContext;
struct HitContext;
struct IDisplayViewport;

//////////////////////////////////////////////////////////////////////////
// Helper Axis object.
//////////////////////////////////////////////////////////////////////////
class EDITOR_COMMON_API CAxisHelper
{
public:
	enum EHelperMode
	{
		HELPER_MODE_NONE   = -1,

		MOVE_MODE          = 0,
		SCALE_MODE         = 1,
		ROTATE_MODE        = 2,
		SELECT_MODE        = 3,
		ROTATE_CIRCLE_MODE = 4,

		MAX_HELPER_MODES   = 5
	};

	enum EHelperFlags
	{
		MOVE_FLAG   = 1,
		SCALE_FLAG  = (1 << SCALE_MODE),
		ROTATE_FLAG = (1 << ROTATE_MODE),
		SELECT_FLAG = (1 << SELECT_MODE),
	};

	enum EAxis
	{
		eAxis_X,
		eAxis_Y,
		eAxis_Z,
	};

	CAxisHelper();

	void SetMode(i32 nModeFlags);
	i32  GetMode() const { return m_nModeFlags; }

	void DrawAxis(const Matrix34& worldTM, const SGizmoPreferences& setup, DisplayContext& dc, float fScaleRatio = 1.0f);
	bool HitTest(const Matrix34& worldTM, const SGizmoPreferences& setup, HitContext& hc, EHelperMode* manipulatorMode = nullptr, float fScaleRatio = 1.0f);

	// 0 - X-Axis, 1 - Y-Axis, 2 - Z-Axis
	void  SetUnchangedAxisForRotationCircle(EAxis axis) { m_UnchangedAxis = axis; }

	void  SetHighlightAxis(i32 axis)                    { m_highlightAxis = axis; };
	i32   GetHighlightAxis() const                      { return m_highlightAxis; };
private:
	void  Prepare(const Matrix34& worldTM, const SGizmoPreferences& setup, IDisplayViewport* view, float fScaleRatio);
	float GetDistance2D(IDisplayViewport* view, CPoint p, Vec3& wp);

	i32      m_nModeFlags;
	i32      m_highlightAxis;
	i32      m_highlightMode;

	float    m_fScreenScale;
	bool     m_bNeedX;
	bool     m_bNeedY;
	bool     m_bNeedZ;
	bool     m_b2D;
	float    m_size;
	Matrix34 m_matrix;
	EAxis    m_UnchangedAxis;
};

#endif // __AxisHelper_h__

