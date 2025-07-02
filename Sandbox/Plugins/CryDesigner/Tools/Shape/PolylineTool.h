// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ShapeTool.h"

namespace Designer
{
enum ELineState
{
	eLineState_Diagonal,
	eLineState_ParallelToAxis,
	eLineState_Cross
};

class PolylineTool : public ShapeTool
{
public:

	PolylineTool(EDesignerTool tool) :
		ShapeTool(tool),
		m_bAlignedToRecentSpotLine(false)
	{
	}

	virtual ~PolylineTool(){}
	virtual void Enter() override;
	virtual void Leave() override;

	bool         OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	bool         OnLButtonUp(CViewport* view, UINT nFlags, CPoint point) override;
	bool         OnMouseMove(CViewport* view, UINT nFlags, CPoint point) override;

	virtual void Display(DisplayContext& dc) override;
	virtual bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;

	bool         IsPhaseFirstStepOnPrimitiveCreation() const override;

protected:

	virtual void CreatePolygonFromSpots(
	  bool bClosePolygon,
	  const SpotList& spotList);

	bool IntersectExisintingLines(
	  const BrushVec3& v0,
	  const BrushVec3& v1,
	  i32* pOutSpotIndex = NULL) const;

	ELineState GetLineState() const;
	void       SetLineState(ELineState lineState);

	ELineState GetAlienedPointWithAxis(
	  const BrushVec3& v0,
	  const BrushVec3& v1,
	  const BrushPlane& plane,
	  IDisplayViewport* view,
	  std::vector<BrushEdge3D>* pAxisList,
	  BrushVec3& outPos) const;

	void AddPolygonWithCurrentSameAsFirst();

	void RegisterEitherEndSpotList();
	void RegisterSpotListAfterBreaking();

	void AlignEdgeWithPrincipleAxises(
	  IDisplayViewport* view,
	  bool bAlign);

	void PutCurrentSpot();
	void Complete();
	void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

private:
	ELineState  m_LineState;

	Spot        m_RecentSpotOnEdge;
	BrushEdge3D m_RecentEdge;
	bool        m_bHasValidRecentEdge;
	bool        m_bAlignedToRecentSpotLine;
	bool        m_bAlignedToAnotherEdge;
};
}

