// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "EditTool.h"

#include <drx3D/CoreX/Sandbox/DrxSignal.h>
#include "Qt/Widgets/QEditToolButton.h"
#include "Gizmos/ITransformManipulator.h"

struct CVegetationInstance;
class CVegetationMap;
class CVegetationObject;

template<typename T>
class QVector;

class CVegetationSelectTool : public CEditTool, public ITransformManipulatorOwner
{
	DECLARE_DYNCREATE(CVegetationSelectTool)

public:
	CVegetationSelectTool();

	virtual string GetDisplayName() const override { return "Select Vegetation"; }
	virtual void   Display(DisplayContext& dc);

	// Overides from CEditTool
	virtual bool MouseCallback(CViewport* pView, EMouseEvent event, CPoint& point, i32 flags);
	virtual void OnManipulatorDrag(IDisplayViewport* pView, ITransformManipulator* pManipulator, const Vec2i& point0, const Vec3& value, i32 nFlags);

	// Key down.
	virtual bool                             OnKeyDown(CViewport* pView, u32 key, u32 repCnt, u32 flags);

	void                                     ClearThingSelection(bool leaveFailedTransformSelected = false);
	void                                     UpdateTransformManipulator();
	void                                     HideSelectedObjects(bool hide);

	bool                                     IsNeedMoveTool() override                 { return true; }
	bool                                     IsNeedToSkipPivotBoxForObjects() override { return true; }
	i32                                      GetCountSelectedInstances() const         { return m_selectedThings.size(); }
	QVector<CVegetationInstance*>            GetSelectedInstances() const;

	void                                     GetManipulatorPosition(Vec3& position) override {};
	bool                                     IsManipulatorVisible() override;

	static QEditToolButtonPanel::SButtonInfo CreateSelectToolButtonInfo();

	CDrxSignal<void()> signalSelectionChanged;
protected:
	virtual ~CVegetationSelectTool();
	// Delete itself.
	virtual void DeleteThis() override { delete this; }

private:
	void                 SelectThing(CVegetationInstance* pThing, bool bShowManipulator = false);
	CVegetationInstance* SelectThingAtPoint(CViewport* pView, CPoint point, bool bNotSelect = false, bool bShowManipulator = false);
	bool                 IsThingSelected(CVegetationInstance* pObj);
	void                 SelectThingsInRect(CViewport* pView, CRect rect, bool bShowManipulator = false);
	void                 MoveSelected(IDisplayViewport* pView, const Vec3& offset);
	void                 ScaleSelected(float scale);
	void                 RotateSelected(const Vec3& rot);

	void                 SetModified(bool withBounds = false, AABB& bounds = AABB(), bool notifySW = true);

	// Specific mouse events handlers.
	bool OnLButtonDown(CViewport* pView, UINT flags, CPoint point);
	bool OnLButtonUp(CViewport* pView, UINT flags, CPoint point);
	bool OnMouseMove(CViewport* pView, UINT flags, CPoint point);

private:
	struct SelectedThing
	{
		SelectedThing(CVegetationInstance* pInstance)
			: pVegetationInstance(pInstance)
			, transformFailed(false)
		{}

		CVegetationInstance* pVegetationInstance;
		bool                 transformFailed;

	private:
		SelectedThing() {}
	};

	enum OpMode
	{
		OPMODE_NONE,
		OPMODE_SELECT,
		OPMODE_MOVE,
		OPMODE_SCALE,
		OPMODE_ROTATE
	};
	OpMode                     m_opMode;
	CPoint                     m_mouseDownPos;
	Vec3                       m_pointerPos;
	//! Selected vegetation instances
	std::vector<SelectedThing> m_selectedThings;
	CVegetationMap*            m_pVegetationMap;
	ITransformManipulator*     m_pManipulator;
};

