// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "AreaSolidTool.h"
#include "Objects/AreaSolidObject.h"
#include "Tools/BaseTool.h"
#include "ToolFactory.h"

namespace Designer
{

// General area solid tool. Used for editing area solids

IMPLEMENT_DYNCREATE(AreaSolidTool, DesignerEditor)

class AreaSolidTool_ClassDesc : public IClassDesc
{
	virtual ESystemClassID SystemClassID() override   { return ESYSTEM_CLASS_EDITTOOL; }
	virtual tukk    ClassName() override       { return "EditTool.AreaSolidTool"; };
	virtual tukk    Category() override        { return "Brush"; };
	virtual CRuntimeClass* GetRuntimeClass() override { return RUNTIME_CLASS(AreaSolidTool); }
};

REGISTER_CLASS_DESC(AreaSolidTool_ClassDesc);

AreaSolidTool::AreaSolidTool()
	: DesignerEditor()
{
	m_Tool = eDesigner_Box;
}

bool AreaSolidTool::OnKeyDown(CViewport* pView, u32 nChar, u32 nRepCnt, u32 nFlags)
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
	if (pTool == nullptr)
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

// Area Solid creation tool

IMPLEMENT_DYNCREATE(CreateAreaSolidTool, AreaSolidTool)

class CreateAreaSolidTool_ClassDesc : public IClassDesc
{
	virtual ESystemClassID SystemClassID() override { return ESYSTEM_CLASS_EDITTOOL; }
	virtual tukk    ClassName() override { return "EditTool.CreateAreaSolidTool"; };
	virtual tukk    Category() override { return "Object"; };
	virtual CRuntimeClass* GetRuntimeClass() override { return RUNTIME_CLASS(CreateAreaSolidTool); }
};

REGISTER_CLASS_DESC(CreateAreaSolidTool_ClassDesc);

CreateAreaSolidTool::CreateAreaSolidTool()
{
	StartCreation("AreaSolid");
}

} //endns Designer

