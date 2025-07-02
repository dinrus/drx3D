// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "Pivot2BottomTool.h"
#include "Core/Model.h"
#include "DesignerEditor.h"
#include "Core/Helper.h"

namespace Designer
{
void Pivot2BottomTool::Enter()
{
	std::vector<MainContext> selections;
	GetSelectedObjectList(selections);

	CUndo undo("Designer : Pivot2Bottom");
	for (i32 i = 0, iCount(selections.size()); i < iCount; ++i)
	{
		if (!selections[i].pModel->CheckFlag(eModelFlag_Mirror))
		{
			selections[i].pModel->RecordUndo("Designer : Pivot2Bottom", selections[i].pObject);
			PivotToCenter(selections[i]);
			Designer::ApplyPostProcess(selections[i], ePostProcess_ExceptMirror);
		}
		else if (iCount == 1)
		{
			MessageBox("Warning", "This tool can't be used in Mirror mode");
		}
	}

	// TODO: Not a modal tool - refactor at some point
	GetDesigner()->SwitchTool(eDesigner_Invalid);
}
}
REGISTER_DESIGNER_TOOL_AND_COMMAND(eDesigner_Pivot2Bottom, eToolGroup_Misc, "Pivot2Bottom", Pivot2BottomTool,
                                   pivot2bottom, "runs pivot2bottom tool", "designer.pivot2bottom")

