// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class InvertSelectionTool : public BaseTool
{
public:

	InvertSelectionTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;

	static void InvertSelection(MainContext& mc);
};
}

