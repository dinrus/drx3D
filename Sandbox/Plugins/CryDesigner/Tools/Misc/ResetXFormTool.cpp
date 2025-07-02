// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ResetXFormTool.h"
#include "Core/Model.h"
#include "DesignerEditor.h"
#include "Core/Helper.h"
#include "Serialization/Decorators/EditorActionButton.h"

using Serialization::ActionButton;

namespace Designer
{
void ResetXFormTool::FreezeXForm(i32 nResetFlag)
{
	std::vector<MainContext> selections;
	GetSelectedObjectList(selections);

	CUndo undo("Designer : ResetXForm");
	for (i32 i = 0, iCount(selections.size()); i < iCount; ++i)
		FreezeXForm(selections[i], nResetFlag);
}

void ResetXFormTool::FreezeXForm(MainContext& mc, i32 nResetFlag)
{
	mc.pModel->RecordUndo("Designer : ResetXForm", mc.pObject);
	ResetXForm(mc, nResetFlag);
	Designer::ApplyPostProcess(mc);
}

void ResetXFormTool::Serialize(Serialization::IArchive& ar)
{
	if (ar.openBlock("Position", " "))
	{
		ar(m_Params.bResetPosition, "Position", "^^Position");
		ar.closeBlock();
	}
	if (ar.openBlock("Rotation", " "))
	{
		ar(m_Params.bResetRotation, "Rotation", "^^Rotation");
		ar.closeBlock();
	}
	if (ar.openBlock("Scale", " "))
	{
		ar(m_Params.bResetScale, "Scale", "^^Scale");
		ar.closeBlock();
	}
	if (ar.openBlock("Reset X Form", " "))
	{
		ar(ActionButton([this] { OnResetXForm();
		                }), "ResetXForm", "^^Reset X Form");
		ar.closeBlock();
	}
}

void ResetXFormTool::OnResetXForm()
{
	i32 nResetFlag = 0;

	if (m_Params.bResetPosition)
		nResetFlag |= eResetXForm_Position;

	if (m_Params.bResetRotation)
		nResetFlag |= eResetXForm_Rotation;

	if (m_Params.bResetScale)
		nResetFlag |= eResetXForm_Scale;

	if (nResetFlag)
		FreezeXForm(nResetFlag);

	if (GetDesigner())
		GetDesigner()->SwitchToSelectTool();
}
}

REGISTER_DESIGNER_TOOL_WITH_PROPERTYTREE_PANEL_AND_COMMAND(eDesigner_ResetXForm, eToolGroup_Misc, "ResetXForm", ResetXFormTool,
                                                           resetxform, "runs ResetXForm tool", "designer.resetxform");

