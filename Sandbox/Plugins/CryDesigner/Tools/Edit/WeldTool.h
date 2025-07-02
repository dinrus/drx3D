// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class WeldTool : public BaseTool
{
public:

	WeldTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	virtual void Enter() override;
	static void  Weld(MainContext& mc, const BrushVec3& vSrc, const BrushVec3& vTarget);
};
}

