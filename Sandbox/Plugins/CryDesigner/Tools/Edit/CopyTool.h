// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class ElementSet;

class CopyTool : public BaseTool
{
public:

	CopyTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void        Enter() override;

	static void Copy(MainContext& mc, ElementSet* pOutCopiedElements);
};
}

