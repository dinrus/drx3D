// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//////////////////////////////////////////////////////////////////////

#ifndef __OPTICSMANAGER_H__
#define __OPTICSMANAGER_H__

#include <drx3D/CoreX/Renderer/IFlares.h>

class COpticsUpr : public DinrusX3dEngBase, public IOpticsUpr
{
public:

	~COpticsUpr(){}

	void                Reset();

	IOpticsElementBase* Create(EFlareType type) const;
	IOpticsElementBase* GetOptics(i32 nIndex);

	bool                Load(tukk fullFlareName, i32& nOutIndex);
	bool                Load(XmlNodeRef& rootNode, i32& nOutIndex);
	bool                AddOptics(IOpticsElementBase* pOptics, tukk name, i32& nOutNewIndex);
	bool                Rename(tukk fullFlareName, tukk newFullFlareName);

	void                GetMemoryUsage(IDrxSizer* pSizer) const;
	void                Invalidate();

private:
	IOpticsElementBase* ParseOpticsRecursively(IOpticsElementBase* pParentOptics, XmlNodeRef& node) const;
	EFlareType          GetFlareType(tukk typeStr) const;
	i32                 FindOpticsIndex(tukk fullFlareName) const;

private:
	std::vector<IOpticsElementBasePtr> m_OpticsList;
	std::map<string, i32>              m_OpticsMap;
	// All flare list which has already been searched for.
	std::set<string>                   m_SearchedOpticsSet;
};

#endif //__OPTICSMANAGER_H__
