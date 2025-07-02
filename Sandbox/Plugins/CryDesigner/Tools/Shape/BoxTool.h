// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "RectangleTool.h"

namespace Designer
{
struct BoxParameter
{
	float m_Height;
	bool  m_bAlignment;

	BoxParameter() :
		m_Height(0),
		m_bAlignment(false)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		if (ar.isEdit())
			ar(LENGTH_RANGE(m_Height), "Height", "Height");

		if (ar.openBlock("Alignment", " "))
		{
			ar(m_bAlignment, "Alignment", "^^Alignment to another polygon");
			ar.closeBlock();
		}
	}
};

class BoxTool : public RectangleTool
{
public:
	BoxTool(EDesignerTool tool) :
		RectangleTool(tool)
	{
	}

	bool         OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	bool         OnLButtonUp(CViewport* view, UINT nFlags, CPoint point) override;
	bool         OnMouseMove(CViewport* view, UINT nFlags, CPoint point) override;
	void         Display(DisplayContext& dc) override;

	virtual void Serialize(Serialization::IArchive& ar) override;

protected:
	void         Register() override;

	virtual void RegisterShape(PolygonPtr pFloorPolygon) override;
	virtual void UpdateShape(bool bUpdateUIs = true) override;
	virtual void UpdateShapeWithBoundaryCheck(bool bUpdateUIs = true) override;

	void         OnLButtonDownAboutPlaceStartingPointPhase(CViewport* view, UINT nFlags, CPoint point) override;

	bool         m_bIsOverOpposite;
	PolygonPtr   m_pCapPolygon;
	BoxParameter m_BoxParameter;
};
}

