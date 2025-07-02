// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class ExportTool : public BaseTool
{
public:

	ExportTool(EDesignerTool tool) : BaseTool(tool)
	{
	}

	void ExportToCgf();
	void ExportToGrp();
	void ExportToObj();

	void Serialize(Serialization::IArchive& ar);
};
}

