// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "RemoveTool.h"
#include "Viewport.h"
#include "Tools/Select/SelectTool.h"
#include "DesignerEditor.h"
#include "Core/Helper.h"
#include "DesignerSession.h"

namespace Designer
{
void RemoveTool::Enter()
{
	__super::Enter();
	RemoveSelectedElements();
	DesignerSession::GetInstance()->signalDesignerEvent(eDesignerNotify_PolygonsModified, nullptr);
	GetDesigner()->SwitchToSelectTool();
}

bool RemoveTool::RemoveSelectedElements(MainContext& mc, bool bEraseMirrored)
{
	ElementSet* pSelected = DesignerSession::GetInstance()->GetSelectedElements();

	if (pSelected->IsEmpty())
		return false;

	bool bRemoved = false;
	i32 postprocessType = ePostProcess_ExceptMirror;

	i32 iSelectedElementCount(pSelected->GetCount());

	for (i32 i = 0; i < iSelectedElementCount; ++i)
	{
		if (!(*pSelected)[i].IsPolygon() || (*pSelected)[i].m_pPolygon == NULL)
			continue;

		i32 nPolygonIndex = -1;
		PolygonPtr pPolygon = mc.pModel->QueryEquivalentPolygon((*pSelected)[i].m_pPolygon, &nPolygonIndex);
		if (!pPolygon)
			continue;
		mc.pModel->RemovePolygon(nPolygonIndex, IsFrameLeftInRemovingPolygon(mc.pObject));
		if (bEraseMirrored)
			MirrorUtil::RemoveMirroredPolygon(mc.pModel, pPolygon);
		bRemoved = true;
	}

	for (i32 i = 0; i < iSelectedElementCount; ++i)
	{
		if (!(*pSelected)[i].IsEdge())
			continue;
		BrushEdge3D edge = (*pSelected)[i].GetEdge();
		if (mc.pModel->EraseEdge(edge))
		{
			mc.pModel->EraseEdge(edge.GetInverted());
			if (bEraseMirrored)
				MirrorUtil::EraseMirroredEdge(mc.pModel, edge);
			bRemoved = true;
		}
	}

	for (i32 i = 0; i < iSelectedElementCount; ++i)
	{
		if (!(*pSelected)[i].IsVertex())
			continue;
		if (mc.pModel->EraseVertex((*pSelected)[i].GetPos()))
		{
			bRemoved = true;
			postprocessType |= ePostProcess_Mirror;
		}
	}

	if (bRemoved)
		Designer::ApplyPostProcess(mc, postprocessType);

	pSelected->Clear();
	return true;
}

bool RemoveTool::RemoveSelectedElements()
{
	CUndo undo("DrxDesigner : Remove elements");
	GetModel()->RecordUndo("Remove elements", GetBaseObject());

	if (RemoveSelectedElements(GetMainContext(), true))
	{
		UpdateSurfaceInfo();
		ApplyPostProcess();
		return true;
	}

	ApplyPostProcess(ePostProcess_GameResource);
	return false;
}
}
REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_Remove, eToolGroup_Edit, "Remove", RemoveTool,
                                   remove, "runs remove tool", "designer.remove")

