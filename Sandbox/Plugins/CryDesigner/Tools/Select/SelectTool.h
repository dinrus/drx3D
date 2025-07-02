// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"
#include "Util/ElementSet.h"

namespace Designer
{
class SelectTool : public BaseTool
{
public:
	SelectTool(EDesignerTool tool) :
		BaseTool(tool),
		m_SelectionType(eST_Nothing),
		m_bHitGizmo(false),
		m_nPickFlag(0),
		m_bAllowSelectionUndo(true)
	{
		if (IsSelectElementMode(tool))
		{
			if (tool & eDesigner_Vertex)
				m_nPickFlag |= ePF_Vertex;
			if (tool & eDesigner_Edge)
				m_nPickFlag |= ePF_Edge;
			if (tool & eDesigner_Polygon)
				m_nPickFlag |= ePF_Polygon;
		}
	}

	virtual ~SelectTool(){}

	virtual bool OnLButtonDown(CViewport* view, UINT nFlags, CPoint point) override;
	virtual bool OnLButtonUp(CViewport* view, UINT nFlags, CPoint point) override;
	virtual bool OnMouseMove(CViewport* view, UINT nFlags, CPoint point) override;
	virtual bool OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;

	virtual void Display(DisplayContext& dc) override;
	virtual void Enter() override;

	virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	void         SelectAllElements();

	void         AddPickFlag(i32 nFlag)    { m_nPickFlag |= nFlag; }
	void         SetPickFlag(i32 nFlag)    { m_nPickFlag = nFlag; }
	void         RemovePickFlag(i32 nFlag) { m_nPickFlag &= ~nFlag; }
	bool         CheckPickFlag(i32 nFlag)  { return m_nPickFlag & nFlag ? true : false; }
	i32          GetPickFlag()             { return m_nPickFlag; }

	void         SetSubMatIDFromLastElement(const ElementSet& elements);

public:

	// first - the index in query result.
	// second - the index in a mark set
	typedef std::pair<i32, i32>               QueryInput;
	typedef std::vector<QueryInput>           QueryInputs;
	// first - polygon index, second - query results of a polygon
	typedef std::map<PolygonPtr, QueryInputs> OrganizedQueryResults;

	static OrganizedQueryResults CreateOrganizedResultsAroundPolygonFromQueryResults(const ModelDB::QueryResult& queryResult);

protected:

	enum eSelectionType
	{
		eST_NormalSelection,
		eST_RectangleSelection,
		eST_EraseSelectionInRectangle,
		eST_JustAboutToTransformSelectedElements,
		eST_TransformSelectedElements,
		eST_Nothing
	};
	void          UpdateCursor(CViewport* view, bool bPickingElements);

	void          SelectElementsInRectangle(CViewport* view, CPoint point, bool bOnlyUseSelectionCube);
	void          EraseElementsInRectangle(CViewport* view, CPoint point, bool bOnlyUseSelectionCube);
	BrushMatrix34 GetOffsetTMOnAlignedPlane(CViewport* pView, const BrushPlane& planeAlighedWithView, CPoint prevPos, CPoint currentPos);
	void          InitalizePlaneAlignedWithView(CViewport* pView);

	eSelectionType m_SelectionType;
	CPoint         m_MouseDownPos;
	ElementSet     m_InitialSelectionElementsInRectangleSel;

	i32            m_nPickFlag;
	bool           m_bHitGizmo;
	bool           m_bAllowSelectionUndo;
	BrushVec3      m_PickedPosAsLMBDown;
	BrushPlane     m_PlaneAlignedWithView;
};
}

