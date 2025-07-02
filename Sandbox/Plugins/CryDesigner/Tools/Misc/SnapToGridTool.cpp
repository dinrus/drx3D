// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SnapToGridTool.h"
#include "Tools/Select/SelectTool.h"
#include "DesignerEditor.h"
#include "Core/PolygonDecomposer.h"
#include "Grid.h"
#include "ViewManager.h"

namespace Designer
{
SnapToGridTool::SnapToGridTool(EDesignerTool tool)
	: BaseTool(tool)
{
}

void SnapToGridTool::Enter()
{
	CUndo undo("Designer : Snap to Grid");
	GetModel()->RecordUndo("Snap To Grid", GetBaseObject());

	ElementSet* pSelected = DesignerSession::GetInstance()->GetSelectedElements();
	for (i32 i = 0, iCount(pSelected->GetCount()); i < iCount; ++i)
	{
		for (i32 k = 0, iVertexCount((*pSelected)[i].m_Vertices.size()); k < iVertexCount; ++k)
		{
			BrushVec3 vSnappedPos = SnapVertexToGrid((*pSelected)[i].m_Vertices[k]);
			(*pSelected)[i].m_Vertices[k] = vSnappedPos;
		}
	}

	ApplyPostProcess();
	GetDesigner()->SwitchToPrevTool();
}

BrushVec3 SnapToGridTool::SnapVertexToGrid(const BrushVec3& vPos)
{
	BrushVec3 vWorldPos = GetWorldTM().TransformPoint(vPos);

	double gridSize = gSnappingPreferences.gridSize();
	double gridScale = gSnappingPreferences.gridScale();

	BrushVec3 vSnappedPos;
	vSnappedPos.x = floor((vWorldPos.x / gridSize) / gridScale + 0.5) * gridSize * gridScale;
	vSnappedPos.y = floor((vWorldPos.y / gridSize) / gridScale + 0.5) * gridSize * gridScale;
	vSnappedPos.z = floor((vWorldPos.z / gridSize) / gridScale + 0.5) * gridSize * gridScale;

	vSnappedPos = GetWorldTM().GetInverted().TransformPoint(vSnappedPos);

	ModelDB::QueryResult qResult;
	GetModel()->GetDB()->QueryAsVertex(vPos, qResult);

	std::vector<PolygonPtr> oldPolygons;
	std::vector<PolygonPtr> newPolygons;

	for (i32 i = 0, iQueryCount(qResult.size()); i < iQueryCount; ++i)
	{
		for (i32 k = 0, iMarkCount(qResult[i].m_MarkList.size()); k < iMarkCount; ++k)
		{
			PolygonPtr pPolygon = qResult[i].m_MarkList[k].m_pPolygon;
			i32 nVertexIndex = qResult[i].m_MarkList[k].m_VertexIndex;

			if (std::abs(pPolygon->GetPlane().Distance(vSnappedPos)) > kDesignerEpsilon)
			{
				oldPolygons.push_back(pPolygon);
				PolygonDecomposer triangulator;
				std::vector<PolygonPtr> triangules;
				triangulator.TriangulatePolygon(pPolygon, triangules);

				for (i32 a = 0, iTriangleCount(triangules.size()); a < iTriangleCount; ++a)
				{
					i32 nVertexIndexInTriangle;
					if (!triangules[a]->GetVertexIndex(vPos, nVertexIndexInTriangle))
						continue;
					triangules[a]->SetPos(nVertexIndexInTriangle, vSnappedPos);
					BrushPlane updatedPlane;
					if (triangules[a]->GetComputedPlane(updatedPlane))
						triangules[a]->SetPlane(updatedPlane);
				}

				newPolygons.insert(newPolygons.end(), triangules.begin(), triangules.end());
			}
			else
			{
				pPolygon->SetPos(nVertexIndex, vSnappedPos);
			}
		}
	}

	for (i32 i = 0, iPolygonCount(oldPolygons.size()); i < iPolygonCount; ++i)
		GetModel()->RemovePolygon(oldPolygons[i]);

	for (i32 i = 0, iPolygonCount(newPolygons.size()); i < iPolygonCount; ++i)
		GetModel()->AddUnionPolygon(newPolygons[i]);

	GetModel()->ResetDB(eDBRF_ALL);

	return vSnappedPos;
}
}

REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_SnapToGrid, eToolGroup_Misc, "Snap to Grid", SnapToGridTool,
                                   snaptogrid, "runs snap to grid tool", "designer.snaptogrid");

