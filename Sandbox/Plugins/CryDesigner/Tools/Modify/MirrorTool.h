// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SliceTool.h"

namespace Designer
{
class MirrorTool : public SliceTool
{
public:

	MirrorTool(EDesignerTool tool) : SliceTool(tool)
	{
	}

	void        OnManipulatorBegin(IDisplayViewport* pView, ITransformManipulator* pManipulator, CPoint& point, i32 flags) override;
	void        OnManipulatorEnd(IDisplayViewport* pView, ITransformManipulator* pManipulator) override;
	void        Display(DisplayContext& dc) override;

	void        OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	void        ApplyMirror();
	void        FreezeModel();

	void        UpdateGizmo() override;
	void        Serialize(Serialization::IArchive& ar);

	static void ReleaseMirrorMode(Model* pModel);
	static void RemoveEdgesOnMirrorPlane(Model* pModel);

private:

	bool UpdateManipulatorInMirrorMode(const BrushMatrix34& offsetTM) override;

};
}

