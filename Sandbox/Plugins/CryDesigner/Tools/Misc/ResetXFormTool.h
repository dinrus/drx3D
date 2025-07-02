// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
struct ResetXFormParams
{
	ResetXFormParams() :
		bResetPosition(true),
		bResetRotation(true),
		bResetScale(true)
	{
	}

	bool bResetPosition;
	bool bResetRotation;
	bool bResetScale;
};

class ResetXFormTool : public BaseTool
{
public:
	ResetXFormTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        FreezeXForm(i32 nResetFlag);
	void        Serialize(Serialization::IArchive& ar);

	static void FreezeXForm(MainContext& mc, i32 nResetFlag);

private:

	void OnResetXForm();

	ResetXFormParams m_Params;
};
}

