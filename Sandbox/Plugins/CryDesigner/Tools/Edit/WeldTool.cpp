// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "WeldTool.h"
#include "Tools/Select/SelectTool.h"
#include "DesignerEditor.h"
#include "Core/PolygonDecomposer.h"

namespace Designer
{
void WeldTool::Weld(MainContext& mc, const BrushVec3& vSrc, const BrushVec3& vTarget)
{
	BrushEdge3D e(vSrc, vTarget);
	std::vector<PolygonPtr> newPolygonList;
	std::vector<PolygonPtr> unnecessaryPolygonList;
	std::vector<std::pair<PolygonPtr, PolygonPtr>> weldedPolygons;

	for (i32 i = 0, iPolygonCount(mc.pModel->GetPolygonCount()); i < iPolygonCount; ++i)
	{
		PolygonPtr pPolygon = mc.pModel->GetPolygon(i);
		DESIGNER_ASSERT(pPolygon);
		if (!pPolygon)
			continue;

		i32 nVertexIndex = -1;
		i32 nEdgeIndex = -1;
		if (pPolygon->HasEdge(e, false, &nEdgeIndex))
		{
			PolygonPtr pWeldedPolygon = pPolygon->Clone();
			IndexPair edgeIndexPair = pWeldedPolygon->GetEdgeIndexPair(nEdgeIndex);
			BrushVec3 v0 = pWeldedPolygon->GetPos(edgeIndexPair.m_i[0]);
			BrushVec3 v1 = pWeldedPolygon->GetPos(edgeIndexPair.m_i[1]);
			if (Comparison::IsEquivalent(e.m_v[0], v1))
			{
				std::swap(edgeIndexPair.m_i[0], edgeIndexPair.m_i[1]);
				std::swap(v0, v1);
			}
			pWeldedPolygon->SetPos(edgeIndexPair.m_i[0], v1);
			pWeldedPolygon->Optimize();
			weldedPolygons.push_back(std::pair<PolygonPtr, PolygonPtr>(pPolygon, pWeldedPolygon));
		}
		else if (pPolygon->HasPosition(e.m_v[0], &nVertexIndex))
		{
			if (std::abs(pPolygon->GetPlane().Distance(e.m_v[1])) > kDesignerEpsilon)
			{
				if (pPolygon->IsQuad())
				{
					PolygonPtr pWeldedPolygon = pPolygon->Clone();
					pWeldedPolygon->SetPos(nVertexIndex, e.m_v[1]);
					pWeldedPolygon->AddFlags(ePolyFlag_NonplanarQuad);
					BrushPlane newPlane;
					pWeldedPolygon->GetComputedPlane(newPlane);
					pWeldedPolygon->SetPlane(newPlane);
					weldedPolygons.push_back(std::pair<PolygonPtr, PolygonPtr>(pPolygon, pWeldedPolygon));
				}
				else
				{
					PolygonDecomposer decomposer;
					std::vector<PolygonPtr> triangulatedPolygons;
					if (decomposer.TriangulatePolygon(pPolygon, triangulatedPolygons))
					{
						unnecessaryPolygonList.push_back(pPolygon);
						for (i32 k = 0, iTriangulatedPolygonCount(triangulatedPolygons.size()); k < iTriangulatedPolygonCount; ++k)
						{
							newPolygonList.push_back(triangulatedPolygons[k]);
							i32 nVertexIndexInSubTri = -1;
							if (!triangulatedPolygons[k]->HasPosition(e.m_v[0], &nVertexIndexInSubTri))
								continue;
							triangulatedPolygons[k]->SetPos(nVertexIndexInSubTri, e.m_v[1]);
							BrushPlane newPlane;
							triangulatedPolygons[k]->GetComputedPlane(newPlane);
							triangulatedPolygons[k]->SetPlane(newPlane);
							triangulatedPolygons[k]->ResetUVs();
						}
					}
				}
			}
			else
			{
				PolygonPtr pWeldedPolygon = pPolygon->Clone();
				pWeldedPolygon->SetPos(nVertexIndex, e.m_v[1]);
				pWeldedPolygon->Optimize();
				weldedPolygons.push_back(std::pair<PolygonPtr, PolygonPtr>(pPolygon, pWeldedPolygon));
			}
		}
	}

	for (i32 i = 0, iUnnecessaryCount(unnecessaryPolygonList.size()); i < iUnnecessaryCount; ++i)
		mc.pModel->RemovePolygon(unnecessaryPolygonList[i]);

	for (i32 i = 0, iNewPolygonCount(newPolygonList.size()); i < iNewPolygonCount; ++i)
		mc.pModel->AddPolygon(newPolygonList[i]);

	for (i32 i = 0, iWeldedPolygonCount(weldedPolygons.size()); i < iWeldedPolygonCount; ++i)
	{
		mc.pModel->RemovePolygon(weldedPolygons[i].first);
		if (weldedPolygons[i].second->IsValid() && !weldedPolygons[i].second->IsOpen())
			mc.pModel->AddPolygon(weldedPolygons[i].second);
	}
}

void WeldTool::Enter()
{
	ElementSet* pSelected = DesignerSession::GetInstance()->GetSelectedElements();

	i32 nSelectionCount = pSelected->GetCount();
	if (nSelectionCount < 2)
	{
		MessageBox("Warning", "More than two vertices should be selected for using this tool.");
		GetDesigner()->SwitchToSelectTool();
		return;
	}

	for (i32 i = 0; i < nSelectionCount; ++i)
	{
		if (!(*pSelected)[i].IsVertex())
		{
			MessageBox("Warning", "Only vertices must be selected for using this tool.");
			GetDesigner()->SwitchToSelectTool();
			return;
		}
	}

	CUndo undo("Designer : Weld");
	GetModel()->RecordUndo("Designer : Weld", GetBaseObject());

	Element lastSelection = (*pSelected)[nSelectionCount - 1];

	for (i32 i = 0; i < nSelectionCount - 1; ++i)
		Weld(GetMainContext(), (*pSelected)[i].GetPos(), (*pSelected)[i + 1].GetPos());

	UpdateSurfaceInfo();
	ApplyPostProcess();

	pSelected->Clear();
	pSelected->Add(lastSelection);
	GetDesigner()->SwitchToSelectTool();
}
}

REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_Weld, eToolGroup_Edit, "Weld", WeldTool,
                                   weld, "runs weld tool", "designer.weld")

