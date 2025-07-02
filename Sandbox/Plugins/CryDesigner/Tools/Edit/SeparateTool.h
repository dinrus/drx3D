// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class DesignerObject;
class SeparateTool : public BaseTool
{
public:

	SeparateTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void                   Enter() override;
	bool                   IsManipulatorVisible() override { return false; }
	static DesignerObject* Separate(MainContext& mc);
};
}

