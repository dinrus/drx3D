// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>

class CObjectArchive;
class CBaseObject;
class CPrefabObject;

class CGuidCollisionResolver
{
public:
	CGuidCollisionResolver(CBaseObject* pObject, CObjectArchive& ar)
		: m_pConflictingObject(pObject)
		, m_archive(ar)
	{}

	bool Resolve();

private:
	std::vector<CBaseObject*> GetPrefabChildren(CPrefabObject* pPrefabObject);

	bool RegenerateGuidsForPrefabHierarchy(CPrefabObject* pPrefabObject);

	bool RegenerateGuidsForConflictingObject(CPrefabObject* pPrefabObject);

	bool RegenerateGuids(CPrefabObject* pTopMostPrefab, CPrefabObject* pPrefabObject);

	void SavePrefabHierarchy(CPrefabObject* pPrefabObject);

	const std::vector<CBaseObject*>& GetSavedChildren(CPrefabObject* pPrefabObject);

	CBaseObject* m_pConflictingObject{ nullptr };
	CObjectArchive& m_archive;
	std::vector<std::pair<CPrefabObject*, std::vector<CBaseObject*>>> m_hierarchy;
};

