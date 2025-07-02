// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Tools/Select/SelectTool.h"

namespace Designer
{
class HidePolygonTool : public SelectTool
{
public:

	HidePolygonTool(EDesignerTool tool) : SelectTool(tool)
	{
		m_nPickFlag = ePF_Polygon;
	}

	void Enter() override;

	bool HideSelectedPolygons();
	void UnhideAll();
	void Serialize(Serialization::IArchive& ar);
};
}

