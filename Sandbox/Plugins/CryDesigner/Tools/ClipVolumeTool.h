// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DesignerEditor.h"

namespace Designer
{
class ClipVolumeTool : public DesignerEditor
{
public:

	DECLARE_DYNCREATE(ClipVolumeTool)
	ClipVolumeTool();
	virtual string      GetDisplayName() const override { return "Clip Volume"; }
	bool                OnKeyDown(CViewport* pView, u32 nChar, u32 nRepCnt, u32 nFlags) override;
};

class CreateClipVolumeTool : public ClipVolumeTool
{
public:
	DECLARE_DYNCREATE(CreateClipVolumeTool)
	CreateClipVolumeTool();
};
}

