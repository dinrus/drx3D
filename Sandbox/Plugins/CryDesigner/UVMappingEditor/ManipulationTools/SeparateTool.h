// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "../BaseTool.h"

namespace Designer {
namespace UVMapping
{
class SeparateTool : public BaseTool
{
public:
	SeparateTool(EUVMappingTool tool);

	void Enter() override;
	void Leave() override;

private:

	void SeparatePolygons();
};
}
}

