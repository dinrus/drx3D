// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "SmoothingGroup.h"

namespace Designer
{
class Model;

class SmoothingGroupManager : public _i_reference_target_t
{
public:

	void                                              Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bUndo, Model* pModel);

	bool                                              AddSmoothingGroup(tukk id_name, SmoothingGroupPtr pSmoothingGroup);
	bool                                              AddPolygon(tukk id_name, PolygonPtr pPolygon);
	void                                              RemoveSmoothingGroup(tukk id_name);
	SmoothingGroupPtr                                 GetSmoothingGroup(tukk id_name) const;
	SmoothingGroupPtr                                 FindSmoothingGroup(PolygonPtr pPolygon) const;
	tukk                                       GetSmoothingGroupID(PolygonPtr pPolygon) const;
	tukk                                       GetSmoothingGroupID(SmoothingGroupPtr pSmoothingGroup) const;

	void                                              InvalidateSmoothingGroup(PolygonPtr pPolygon);
	void                                              InvalidateAll();

	i32                                               GetCount() { return m_SmoothingGroups.size(); }

	std::vector<std::pair<string, SmoothingGroupPtr>> GetSmoothingGroupList() const;

	void                                              Clear();
	void                                              Clean(Model* pModel);
	void                                              RemovePolygon(PolygonPtr pPolygon);

	string                                            GetEmptyGroupID() const;

	void                                              CopyFromModel(Model* pModel, const Model* pSourceModel);

private:

	typedef std::map<string, SmoothingGroupPtr> SmoothingGroupMap;
	typedef std::map<PolygonPtr, string>        InvertSmoothingGroupMap;

	SmoothingGroupMap       m_SmoothingGroups;
	InvertSmoothingGroupMap m_MapPolygon2GroupId;
};
}

