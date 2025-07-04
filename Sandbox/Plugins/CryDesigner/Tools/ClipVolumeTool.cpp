// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "ClipVolumeTool.h"
#include "Core/Helper.h"
#include "Tools/AreaSolidTool.h"
#include "Tools/BaseTool.h"
#include "ToolFactory.h"
#include "Objects/ClipVolumeObject.h"
#include "DesignerSession.h"

namespace Designer
{

// General clip volume tool. Used for editing clip volumes

IMPLEMENT_DYNCREATE(ClipVolumeTool, DesignerEditor)

class ClipVolumeTool_ClassDesc : public IClassDesc
{
	ESystemClassID SystemClassID()   { return ESYSTEM_CLASS_EDITTOOL; }
	tukk    ClassName()       { return "EditTool.ClipVolumeTool"; };
	tukk    Category()        { return "Brush"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(ClipVolumeTool); }
};

REGISTER_CLASS_DESC(ClipVolumeTool_ClassDesc);

ClipVolumeTool::ClipVolumeTool()
	: DesignerEditor()
{
	m_Tool = eDesigner_Box;
}

bool ClipVolumeTool::OnKeyDown(CViewport* pView, u32 nChar, u32 nRepCnt, u32 nFlags)
{
	if (nChar == Qt::Key_Delete)
		return false;

	if (nChar == Qt::Key_Escape)
	{
		if (IsSelectElementMode(m_Tool))
		{
			LeaveCurrentTool();
			return false;
		}
		else if (m_Tool == eDesigner_Pivot)
		{
			SwitchToSelectTool();
			return true;
		}
	}

	BaseTool* pTool = GetTool(m_Tool);
	if (pTool == NULL)
		return false;

	if (pTool->OnKeyDown(pView, nChar, nRepCnt, nFlags))
	{
		if (nChar == Qt::Key_Escape)
		{
			// Release the current tool by ending the current session
			DesignerSession::GetInstance()->EndSession();
		}
		return true;
	}

	return false;
}

// Clip Volume creation tool

IMPLEMENT_DYNCREATE(CreateClipVolumeTool, ClipVolumeTool)

class CreateClipVolumeTool_ClassDesc : public IClassDesc
{
	ESystemClassID SystemClassID() { return ESYSTEM_CLASS_EDITTOOL; }
	tukk    ClassName() { return "EditTool.CreateClipVolumeTool"; };
	tukk    Category() { return "Object"; };

	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CreateClipVolumeTool); }
};

REGISTER_CLASS_DESC(CreateClipVolumeTool_ClassDesc);

CreateClipVolumeTool::CreateClipVolumeTool()
{
	StartCreation("ClipVolume");
}

}

