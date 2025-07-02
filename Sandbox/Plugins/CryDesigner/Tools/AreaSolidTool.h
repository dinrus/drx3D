// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DesignerEditor.h"

namespace Designer
{
class AreaSolidTool : public DesignerEditor
{
public:
	DECLARE_DYNCREATE(AreaSolidTool)
	AreaSolidTool();
	virtual string      GetDisplayName() const override { return "Area Solid"; }
	virtual bool        OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;
};

class CreateAreaSolidTool final : public AreaSolidTool
{
public:

	DECLARE_DYNCREATE(CreateAreaSolidTool)
	CreateAreaSolidTool();
};

} //endns Designer

