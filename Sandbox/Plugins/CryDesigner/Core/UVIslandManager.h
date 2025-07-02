// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "UVIsland.h"

namespace Designer
{
class UVIslandManager : public _i_reference_target_t
{
public:
	void             AddUVIsland(UVIslandPtr pUVIsland);

	bool             FindUVIslandsHavingPolygon(PolygonPtr polygon, std::vector<UVIslandPtr>& OutUVIslands);
	void             RemoveEmptyUVIslands();

	i32              GetCount() const           { return m_UVIslands.size(); }
	UVIslandPtr      GetUVIsland(i32 idx) const { return m_UVIslands[idx]; }
	UVIslandPtr      FindUVIsland(const DrxGUID& guid) const;
	void             Reset(Model* pModel);

	void             Join(UVIslandManager* pUVIslandManager);
	void             ConvertIsolatedAreasToIslands();

	void             Clear() { m_UVIslands.clear(); }
	void             Clean(Model* pModel);
	void             Invalidate();
	bool             HasSubMatID(i32 nSubMatID) const;

	void             Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bUndo, Model* pModel);

	UVIslandManager& operator=(const UVIslandManager& uvIslandMgr);
	UVIslandManager* Clone() const;

private:
	std::vector<UVIslandPtr> m_UVIslands;
};
}

