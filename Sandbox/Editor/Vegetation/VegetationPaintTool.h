// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "EditTool.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include "Qt/Widgets/QEditToolButton.h"

class CVegetationObject;
struct CVegetationInstance;
class CVegetationMap;

class CVegetationPaintTool : public CEditTool
{
	DECLARE_DYNCREATE(CVegetationPaintTool)
public:
	CVegetationPaintTool();

	virtual string GetDisplayName() const override { return "Paint Vegetation"; }
	virtual void   Display(DisplayContext& dc);

	// Overides from CEditTool
	virtual bool MouseCallback(CViewport* pView, EMouseEvent event, CPoint& point, i32 flags) override;

	// Key down.
	virtual bool                             OnKeyDown(CViewport* pView, u32 key, u32 repCnt, u32 flags) override;
	virtual bool                             OnKeyUp(CViewport* pView, u32 key, u32 repCnt, u32 flags) override;

	void                                     SetBrushRadius(float r);
	float                                    GetBrushRadius() const { return m_brushRadius; };

	void                                     PaintBrush();
	void                                     PlaceThing(CViewport* pView, const CPoint& point);

	static QEditToolButtonPanel::SButtonInfo CreatePaintToolButtonInfo();
	static float                             GetMinBrushRadius();
	static float                             GetMaxBrushRadius();

	static void                              Command_Activate(bool activateEraseMode);

	CDrxSignal<void()> signalBrushRadiusChanged;

protected:
	virtual ~CVegetationPaintTool() {}
	// Delete itself.
	virtual void DeleteThis() override { delete this; }

protected:
	bool m_eraseMode;
	bool m_isEraseTool;
	bool m_isModeSwitchAllowed;

private:
	void SetModified(AABB& bounds, bool notifySW = true);

	// Specific mouse events handlers.
	void OnLButtonDown(CViewport* pView, i32 flags, const CPoint& point);
	void OnLButtonUp();
	void OnMouseMove(CViewport* pView, i32 flags, const CPoint& point);
	void SetBrushRadiusWithSignal(float radius);

private:
	Vec3               m_pointerPos;
	CPoint             m_prevMousePos;
	CVegetationMap*    m_vegetationMap;
	bool               m_isAffectedByBrushes;
	float              m_brushRadius;
	static const float s_minBrushRadius;
	static const float s_maxBrushRadius;
};

