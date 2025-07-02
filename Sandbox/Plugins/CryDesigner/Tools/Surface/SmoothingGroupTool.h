// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/Select/SelectTool.h"
#include "Core/SmoothingGroupManager.h"

namespace Designer
{
class SmoothingGroupTool : public SelectTool
{
public:
	SmoothingGroupTool(EDesignerTool tool) : SelectTool(tool)
	{
		m_nPickFlag = ePF_Polygon;
		m_fThresholdAngle = 45.0f;
	}

	void                                              Enter() override;

	bool                                              OnKeyDown(CViewport* view, u32 nChar, u32 nRepCnt, u32 nFlags) override;
	void                                              OnEditorNotifyEvent(EEditorNotifyEvent event) override;

	void                                              AddSmoothingGroup();
	void                                              AddPolygonsToSmoothingGroup(tukk name);
	void                                              SelectPolygons(tukk name);
	void                                              DeleteSmoothingGroup(tukk name);
	bool                                              RenameSmoothingGroup(tukk former_name, tukk name);
	void                                              ApplyAutoSmooth();

	void                                              SetThresholdAngle(float thresholdAngle) { m_fThresholdAngle = thresholdAngle; }
	float                                             GetThresholdAngle() const               { return m_fThresholdAngle; }

	std::vector<std::pair<string, SmoothingGroupPtr>> GetSmoothingGroupList() const;

private:
	void ClearSelectedElements();
	float m_fThresholdAngle;
};
}

