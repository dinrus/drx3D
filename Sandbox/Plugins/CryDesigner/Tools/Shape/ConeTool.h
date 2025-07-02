// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "CylinderTool.h"

namespace Designer
{
class ConeTool : public CylinderTool
{
public:

	ConeTool(EDesignerTool tool) : CylinderTool(tool)
	{
	}
	~ConeTool(){}

private:

	void UpdateShape(float fHeight) override;
};
}

