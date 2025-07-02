// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DiscTool.h"

namespace Designer
{
class SphereTool : public DiscTool
{
public:
	SphereTool(EDesignerTool tool) :
		DiscTool(tool),
		m_MatTo001(BrushMatrix34::CreateIdentity())
	{
	}
	~SphereTool(){}

protected:
	void UpdateShape() override;
	void UpdateDesignerBasedOnSpherePolygons(const BrushMatrix34& tm);
	void Register() override;

private:
	BrushMatrix34           m_MatTo001;
	std::vector<PolygonPtr> m_SpherePolygons;
};
}

