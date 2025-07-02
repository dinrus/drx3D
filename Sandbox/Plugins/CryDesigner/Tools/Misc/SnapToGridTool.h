// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/BaseTool.h"

namespace Designer
{
class SnapToGridTool : public BaseTool
{
public:

	SnapToGridTool(EDesignerTool tool);

	void      Enter() override;

	BrushVec3 SnapVertexToGrid(const BrushVec3& vPos);
};
}

