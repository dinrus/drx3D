// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ShapeTool.h"

namespace Designer
{
enum EStairHeightCalculationWayMode
{
	eStairHeightCalculationWay_StepRise,
	eStairHeightCalculationWay_StepNumber
};

struct StairProfileParameter
{
	float m_StepRise;
	bool  m_bClosedProfile;

	StairProfileParameter() :
		m_StepRise(kDefaultStepRise),
		m_bClosedProfile(true)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		ar(STEPRISE_RANGE(m_StepRise), "StepRise", "Step Rise");
		ar(m_bClosedProfile, "Closed Profile", "^^Closed Profile");
	}
};

enum ESideStairMode
{
	eSideStairMode_PlaceFirstPoint,
	eSideStairMode_DrawDiagonal,
	eSideStairMode_SelectDirection,
};

class StairProfileTool : public ShapeTool
{
public:

	StairProfileTool(EDesignerTool tool) :
		ShapeTool(tool),
		m_SideStairMode(eSideStairMode_PlaceFirstPoint),
		m_nSelectedCandidate(0)
	{
	}

	void Enter() override;

	bool OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	bool OnMouseMove(CViewport* view, UINT nFlags, CPoint point) override;
	bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;
	void Display(DisplayContext& dc) override;
	void Serialize(Serialization::IArchive& ar) override;
	bool IsPhaseFirstStepOnPrimitiveCreation() const override;

protected:

	void CreateCandidates();
	void DrawCandidateStair(DisplayContext& dc, i32 nIndex, const ColorB& color);
	Spot Convert2Spot(Model* pModel, const BrushPlane& plane, const BrushVec2& pos) const;

	ESideStairMode        m_SideStairMode;
	std::vector<Spot>     m_CandidateStairs[2];
	BrushLine             m_BorderLine;
	i32                   m_nSelectedCandidate;
	Spot                  m_LastSpot;
	i32                   m_nSelectedPolygonIndex;
	StairProfileParameter m_StairProfileParameter;
};
}

