// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
enum EPivotSelectionType
{
	ePST_BoundBox,
	ePST_Mesh,
};

class PivotTool : public BaseTool
{
public:
	PivotTool(EDesignerTool tool) : BaseTool(tool), m_PivotSelectionType(ePST_BoundBox)
	{
	}

	void Enter() override;
	void Leave() override;

	void Display(DisplayContext& dc) override;

	bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;
	bool OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	bool OnMouseMove(CViewport* view, UINT nFlags, CPoint point) override;

	void SetSelectionType(EPivotSelectionType selectionType, bool bForce = false);

	void OnManipulatorDrag(IDisplayViewport* pView, ITransformManipulator* pManipulator, CPoint& p0, BrushVec3 value, i32 flags) override;
	void OnManipulatorBegin(IDisplayViewport* pView, ITransformManipulator* pManipulator, CPoint& point, i32 flags) override;

	void Serialize(Serialization::IArchive& ar);

private:

	void Confirm();
	void InitializeManipulator();

	std::vector<BrushVec3> m_CandidateVertices;
	i32                    m_nSelectedCandidate;
	i32                    m_nPivotIndex;
	BrushVec3              m_PivotPos;
	BrushVec3              m_StartingDragManipulatorPos;
	EPivotSelectionType    m_PivotSelectionType;
};
}

