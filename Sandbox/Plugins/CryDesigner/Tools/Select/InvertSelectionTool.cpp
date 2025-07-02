// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "InvertSelectionTool.h"
#include "DesignerEditor.h"
#include "Util/ElementSet.h"
#include "Tools/Select/SelectTool.h"

namespace Designer
{
void InvertSelectionTool::InvertSelection(MainContext& mc)
{
	i32 nSelectedElementCount = mc.pSelected->GetCount();
	ElementSet newSelectionList;
	for (i32 k = 0, iPolygonCount(mc.pModel->GetPolygonCount()); k < iPolygonCount; ++k)
	{
		bool bSameExist = false;
		PolygonPtr pPolygon = mc.pModel->GetPolygon(k);
		if (pPolygon->CheckFlags(ePolyFlag_Hidden))
			continue;

		for (i32 i = 0; i < nSelectedElementCount; ++i)
		{
			if (!(*mc.pSelected)[i].IsPolygon() || (*mc.pSelected)[i].m_pPolygon == NULL)
				continue;
			if ((*mc.pSelected)[i].m_pPolygon == pPolygon)
			{
				bSameExist = true;
				break;
			}
		}
		if (!bSameExist)
		{
			Element de;
			de.SetPolygon(mc.pObject, pPolygon);
			newSelectionList.Add(de);
		}
	}

	mc.pSelected->Clear();
	mc.pSelected->Add(newSelectionList);
}

void InvertSelectionTool::Enter()
{
	CUndo undo("Designer : Invert Selection");
	GetDesigner()->StoreSelectionUndo();
	InvertSelection(GetMainContext());
	DesignerSession* pSession = DesignerSession::GetInstance();
	pSession->UpdateSelectionMeshFromSelectedElements();
	GetDesigner()->SwitchToPrevTool();
	pSession->signalDesignerEvent(eDesignerNotify_Select, nullptr);
}
}

REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_Invert, eToolGroup_Selection, "Invert", InvertSelectionTool,
                                   invertselection, "runs invert selection tool", "designer.invertselection")

