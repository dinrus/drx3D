// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "PolylineTool.h"

namespace Designer
{
struct CurveParameter
{
	CurveParameter() :
		m_NumOfSubdivision(kDefaultSubdivisionNum)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		ar(SUBDIVISION_RANGE(m_NumOfSubdivision), "SubdivisionCount", "Subdivision Count");
	}

	i32 m_NumOfSubdivision;
};

class CurveTool : public PolylineTool
{
public:

	CurveTool(EDesignerTool tool) :
		PolylineTool(tool),
		m_ArcState(eArcState_ChooseFirstPoint)
	{
	}

	virtual ~CurveTool(){}

	void Leave();
	bool OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) { return true; }
	bool OnLButtonUp(CViewport* view, UINT nFlags, CPoint point);
	bool OnMouseMove(CViewport* view, UINT nFlags, CPoint point);
	void Display(DisplayContext& dc);
	bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags);

	bool IsPhaseFirstStepOnPrimitiveCreation() const override;
	void Serialize(Serialization::IArchive& ar) override;

protected:
	void PrepareArcSpots(CViewport* view, UINT nFlags, CPoint point);

	enum EDrawingArcState
	{
		eArcState_ChooseFirstPoint,
		eArcState_ChooseLastPoint,
		eArcState_ControlMiddlePoint
	};

	ELineState       m_LineState;
	EDrawingArcState m_ArcState;
	Spot             m_LastSpot;

	CurveParameter   m_CurveParameter;
};
}

