// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "LoopSelectionTool.h"
#include "DesignerEditor.h"
#include "Util/ElementSet.h"
#include "Core/LoopPolygons.h"
#include "DesignerSession.h"

namespace Designer
{
i32 s_nMaximumRepeatCount = 0;

void LoopSelectionTool::LoopSelection(MainContext& mc)
{
	ElementSet copiedSelected = *mc.pSelected;
	i32 nSelectedElementCount = copiedSelected.GetCount();

	if (nSelectedElementCount >= 2)
	{
		const Element& firstElement = copiedSelected[nSelectedElementCount - 2];
		const Element& secondElement = copiedSelected[nSelectedElementCount - 1];
		if (firstElement.IsPolygon() && secondElement.IsPolygon())
		{
			SelectPolygonLoop(mc, firstElement.m_pPolygon, secondElement.m_pPolygon);
			SelectPolygonLoop(mc, secondElement.m_pPolygon, firstElement.m_pPolygon);
			return;
		}
	}

	mc.pSelected->Clear();
	s_nMaximumRepeatCount = CountAllEdges(mc) * 50;

	for (i32 i = 0; i < nSelectedElementCount; ++i)
	{
		if (!copiedSelected[i].IsEdge())
			continue;
		if (!SelectLoop(mc, copiedSelected[i].GetEdge()))
		{
			mc.pSelected->Clear();

			if (SelectBorderInOnePolygon(mc, copiedSelected[i].GetEdge()))
				continue;

			ElementSet elements;
			if (SelectBorder(mc, copiedSelected[i].GetEdge(), elements))
			{
				mc.pSelected->Add(elements);
				mc.pSelected->Add(Element(mc.pObject, copiedSelected[i].GetEdge()));
			}
		}
	}
}

void AddEdgeToList(std::vector<BrushEdge3D>& edgeList, const BrushEdge3D& edge)
{
	BrushEdge3D invEdge = edge.GetInverted();

	for (i32 i = 0, iEdgeCount(edgeList.size()); i < iEdgeCount; ++i)
	{
		if (edgeList[i].IsEquivalent(edge) || edgeList[i].IsEquivalent(invEdge))
			return;
	}

	edgeList.push_back(edge);
}

void LoopSelectionTool::Enter()
{
	LoopSelection(GetMainContext());
	GetDesigner()->SwitchToPrevTool();
	DesignerSession::GetInstance()->signalDesignerEvent(eDesignerNotify_Select, nullptr);
}

bool HasEdge(std::vector<PolygonPtr>& polygonList, const BrushEdge3D& edge)
{
	for (i32 i = 0, iPolygonCount(polygonList.size()); i < iPolygonCount; ++i)
	{
		if (polygonList[i]->HasEdge(edge))
			return true;
	}
	return false;
}

bool FindNextEdge(MainContext& mc, const BrushEdge3D& edge, BrushEdge3D& outEdge)
{
	ModelDB* pDB = mc.pModel->GetDB();
	ModelDB::QueryResult queryResult;

	if (!pDB->QueryAsVertex(edge.m_v[1], queryResult))
		return false;

	if (queryResult.size() != 1)
		return false;

	if (queryResult[0].m_MarkList.size() != 4)
		return false;

	std::vector<PolygonPtr> invalidPolygons;
	std::vector<PolygonPtr> validPolygons;
	for (i32 i = 0; i < 4; ++i)
	{
		PolygonPtr pPolygon = queryResult[0].m_MarkList[i].m_pPolygon;
		if (pPolygon->HasEdge(edge))
			invalidPolygons.push_back(pPolygon);
		else
			validPolygons.push_back(pPolygon);
	}

	for (i32 i = 0, iPolygonCount(validPolygons.size()); i < iPolygonCount; ++i)
	{
		PolygonPtr pPolygon = validPolygons[i];
		for (i32 k = 0, iEdgeCount(pPolygon->GetEdgeCount()); k < iEdgeCount; ++k)
		{
			BrushEdge3D e = pPolygon->GetEdge(k);
			if (Comparison::IsEquivalent(e.m_v[0], edge.m_v[1]) && !HasEdge(invalidPolygons, e))
			{
				outEdge = e;
				return true;
			}
		}
	}

	return false;
}

bool LoopSelectionTool::SelectLoop(MainContext& mc, const BrushEdge3D& initialEdge)
{
	BrushEdge3D edge = initialEdge;
	ElementSet edgeElements;

	i32 nCount = 0;
	bool bLoop = false;

	for (i32 i = 0; i < 2; ++i)
	{
		while (!bLoop)
		{
			edgeElements.Add(Element(mc.pObject, edge));
			BrushEdge3D nextEdge;
			if (!FindNextEdge(mc, edge, nextEdge))
				break;
			bLoop = nextEdge.IsEquivalent(initialEdge);
			edge = nextEdge;
		}
		if (bLoop)
			break;
		edge = initialEdge.GetInverted();
	}

	if (edgeElements.IsEmpty() || (edgeElements.GetCount() == 1 && edgeElements[0].IsEdge() && edgeElements[0].GetEdge().IsEquivalent(initialEdge)))
		return false;

	ElementSet* pSelected = DesignerSession::GetInstance()->GetSelectedElements();
	pSelected->Add(edgeElements);

	return true;
}

bool LoopSelectionTool::SelectBorderInOnePolygon(MainContext& mc, const BrushEdge3D& edge)
{
	ElementSet* pSelected = DesignerSession::GetInstance()->GetSelectedElements();

	std::vector<PolygonPtr> adjacentPolygons;
	mc.pModel->QueryPolygonsSharingEdge(edge, adjacentPolygons);

	if (adjacentPolygons.size() == 1)
	{
		std::vector<PolygonPtr> outerPolygons = adjacentPolygons[0]->GetLoops()->GetOuterClones();
		for (i32 i = 0, outerPolygonCount(outerPolygons.size()); i < outerPolygonCount; ++i)
		{
			if (!outerPolygons[i]->HasEdge(edge))
				continue;
			i32 nAddedCount = 0;
			i32 iEdgeCount(outerPolygons[i]->GetEdgeCount());
			for (i32 k = 0; k < iEdgeCount; ++k)
			{
				BrushEdge3D e = outerPolygons[i]->GetEdge(k);
				if (e.IsEquivalent(edge))
					continue;
				if (GetPolygonCountSharingEdge(mc, e) == 1)
				{
					++nAddedCount;
					pSelected->Add(Element(mc.pObject, e));
				}
			}
			if (nAddedCount != iEdgeCount)
			{
				pSelected->Clear();
				return false;
			}
			return true;
		}
	}

	for (i32 a = 0, iAdjacentPolygonCount(adjacentPolygons.size()); a < iAdjacentPolygonCount; ++a)
	{
		std::vector<PolygonPtr> innterPolygons = adjacentPolygons[a]->GetLoops()->GetHoleClones();
		for (i32 i = 0, innerPolygonCount(innterPolygons.size()); i < innerPolygonCount; ++i)
		{
			if (!innterPolygons[i]->HasEdge(edge))
				continue;
			for (i32 k = 0, iEdgeCount(innterPolygons[i]->GetEdgeCount()); k < iEdgeCount; ++k)
			{
				BrushEdge3D e = innterPolygons[i]->GetEdge(k);
				if (GetPolygonCountSharingEdge(mc, e, &(innterPolygons[i]->GetPlane())) == 1)
					pSelected->Add(Element(mc.pObject, e));
			}
			return true;
		}
	}

	return false;
}

bool LoopSelectionTool::SelectBorder(MainContext& mc, const BrushEdge3D& edge, ElementSet& outElementInfos)
{
	BrushEdge3D e(edge);
	i32 nCount = 0;

	while (nCount++ < s_nMaximumRepeatCount)
	{
		std::vector<BrushEdge3D> edgeList;

		if (!mc.pModel->QueryEdgesWithPos(e.m_v[1], edgeList))
			return true;

		bool bAdded = false;
		for (i32 i = 0, iEdgeCount(edgeList.size()); i < iEdgeCount; ++i)
		{
			if (e.IsEquivalent(edgeList[i]) || e.GetInverted().IsEquivalent(edgeList[i]))
				continue;
			if (GetPolygonCountSharingEdge(mc, edgeList[i]) != 1)
				continue;
			if (!outElementInfos.Add(Element(mc.pObject, edgeList[i])))
				return true;
			e = edgeList[i];
			bAdded = true;
			break;
		}
		if (!bAdded)
			break;

		if (Comparison::IsEquivalent(e.m_v[1], edge.m_v[0]))
			return true;
	}

	return false;
}

i32 LoopSelectionTool::GetPolygonCountSharingEdge(MainContext& mc, const BrushEdge3D& edge, const BrushPlane* pPlane)
{
	std::vector<PolygonPtr> polygons;
	mc.pModel->QueryPolygonsSharingEdge(edge, polygons);
	i32 nCount = 0;
	for (i32 i = 0, iCount(polygons.size()); i < iCount; ++i)
	{
		if (!pPlane || polygons[i]->GetPlane().IsEquivalent(*pPlane))
			++nCount;
	}
	return nCount;
}

i32 LoopSelectionTool::CountAllEdges(MainContext& mc)
{
	i32 nEdgeCount = 0;
	for (i32 i = 0, iPolygonCount(mc.pModel->GetPolygonCount()); i < iPolygonCount; ++i)
		nEdgeCount += mc.pModel->GetPolygon(i)->GetEdgeCount();
	return nEdgeCount;
}

bool LoopSelectionTool::SelectPolygonLoop(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon)
{
	std::vector<PolygonPtr> polygons;
	GetLoopPolygons(mc, pFirstPolygon, pSecondPolygon, polygons);

	if (polygons.empty())
		return false;

	for (i32 i = 0, iPolygonCount(polygons.size()); i < iPolygonCount; ++i)
		mc.pSelected->Add(Element(mc.pObject, polygons[i]));

	return true;
}

bool LoopSelectionTool::GetLoopPolygons(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon, std::vector<PolygonPtr>& outPolygons)
{
	bool bAdded = false;
	i32 nCount = 0;

	PolygonPtr pInitPolygon = pFirstPolygon;

	while (pSecondPolygon && pInitPolygon != pSecondPolygon && pSecondPolygon->GetEdgeCount() == 4 && (nCount++) < mc.pModel->GetPolygonCount())
	{
		PolygonPtr pAdjacentPolygon = FindAdjacentNextPolygon(mc, pFirstPolygon, pSecondPolygon);
		pFirstPolygon = pSecondPolygon;
		pSecondPolygon = pAdjacentPolygon;
		if (pAdjacentPolygon && pAdjacentPolygon != pInitPolygon)
		{
			bAdded = true;
			outPolygons.push_back(pAdjacentPolygon);
		}
	}

	return bAdded;
}

bool LoopSelectionTool::GetLoopPolygonsInBothWays(MainContext& mc, PolygonPtr pFirstPolygon, PolygonPtr pSecondPolygon, std::vector<PolygonPtr>& outPolygons)
{
	std::vector<PolygonPtr> polygonOneWay;
	std::vector<PolygonPtr> polygonTheOtherWay;
	bool bAdded0 = GetLoopPolygons(mc, pFirstPolygon, pSecondPolygon, polygonOneWay);
	bool bAdded1 = GetLoopPolygons(mc, pSecondPolygon, pFirstPolygon, polygonTheOtherWay);

	if (!bAdded0 && !bAdded1)
		return false;

	if (!polygonOneWay.empty())
		outPolygons.insert(outPolygons.end(), polygonOneWay.begin(), polygonOneWay.end());

	auto ii = polygonTheOtherWay.begin();
	for (; ii != polygonTheOtherWay.end(); )
	{
		bool bErased = false;
		for (i32 k = 0, iPolygonCount(polygonOneWay.size()); k < iPolygonCount; ++k)
		{
			if (polygonOneWay[k] == *ii)
			{
				ii = polygonTheOtherWay.erase(ii);
				bErased = true;
				break;
			}
		}
		if (!bErased)
			++ii;
	}

	if (!polygonTheOtherWay.empty())
		outPolygons.insert(outPolygons.begin(), polygonTheOtherWay.rbegin(), polygonTheOtherWay.rend());

	return true;
}

PolygonPtr LoopSelectionTool::FindAdjacentNextPolygon(MainContext& mc, PolygonPtr pFristPolygon, PolygonPtr pSecondPolygon)
{
	i32 nFirstEdgeCount = pFristPolygon->GetEdgeCount();
	i32 nSecondEdgeCount = pSecondPolygon->GetEdgeCount();

	if ((nFirstEdgeCount != 3 && nFirstEdgeCount != 4) || (nSecondEdgeCount != 3 && nSecondEdgeCount != 4))
		return NULL;

	std::vector<Vertex> firstVs;
	std::vector<Vertex> secondVs;
	pFristPolygon->GetLinkedVertices(firstVs);
	pSecondPolygon->GetLinkedVertices(secondVs);
	for (i32 i = 0; i < nFirstEdgeCount; ++i)
	{
		BrushEdge3D first_e(firstVs[i].pos, firstVs[(i + 1) % nFirstEdgeCount].pos);
		for (i32 k = 0; k < nSecondEdgeCount; ++k)
		{
			BrushEdge3D second_e(secondVs[(k + 1) % nSecondEdgeCount].pos, secondVs[k].pos);
			if (first_e.IsEquivalent(second_e))
			{
				if (nSecondEdgeCount != 4)
					continue;
				i32 nSecondEdgeCount_Half = nSecondEdgeCount / 2;
				BrushEdge3D oppositeEdge(secondVs[(k + nSecondEdgeCount_Half) % nSecondEdgeCount].pos, secondVs[(k + nSecondEdgeCount_Half + 1) % nSecondEdgeCount].pos);
				std::vector<PolygonPtr> polygonsSharingEdge;
				mc.pModel->QueryPolygonsSharingEdge(oppositeEdge, polygonsSharingEdge);
				i32 nPolygonCount(polygonsSharingEdge.size());
				for (i32 a = 0; a < nPolygonCount; ++a)
				{
					if (polygonsSharingEdge[a]->CheckFlags(ePolyFlag_Hidden))
						continue;
					i32 nEdgeCount = polygonsSharingEdge[a]->GetEdgeCount();
					if (polygonsSharingEdge[a] != pSecondPolygon && (nEdgeCount == 3 || nEdgeCount == 4))
						return polygonsSharingEdge[a];
				}
			}
		}
	}
	return NULL;
}
}

REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_Loop, eToolGroup_Selection, "Loop", LoopSelectionTool,
                                   loopselection, "runs loop selection tool", "designer.loopselection")

