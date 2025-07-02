// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class BooleanTool : public BaseTool
{
public:

	BooleanTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void Enter() override;

	void BooleanOperation(EBooleanOperationEnum booleanType);
	void Serialize(Serialization::IArchive& ar);

	void Union()        { BooleanOperation(eBOE_Union); }
	void Subtract()     { BooleanOperation(eBOE_Difference); }
	void Intersection() { BooleanOperation(eBOE_Intersection); }
};
}

