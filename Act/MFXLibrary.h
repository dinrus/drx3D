// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ----------------------------------------------------------------------------------------
//  Имя файла:   MFXLibrary.h
//  Описание: A library which contains a set of MFXEffects
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MFX_LIBRARY_H_
#define _MFX_LIBRARY_H_

#pragma once

#include "MFXContainer.h"

class CMFXLibrary
{
private:
	typedef std::map<TMFXNameId, TMFXContainerPtr, stl::less_stricmp<string>> TEffectContainersMap;

public:
	typedef std::vector<string> TEffectNames;

	struct SLoadingEnvironment
	{
	public:

		SLoadingEnvironment(const TMFXNameId& _libraryName, const XmlNodeRef& _libraryParamsNode, std::vector<TMFXContainerPtr>& _allContainers)
			: libraryName(_libraryName)
			, libraryParamsNode(_libraryParamsNode)
			, allContainers(_allContainers)
		{
		}

		void AddLibraryContainer(const TMFXNameId& libraryName, TMFXContainerPtr pContainer)
		{
			pContainer->SetLibraryAndId(libraryName, static_cast<TMFXEffectId>(allContainers.size()));
			allContainers.push_back(pContainer);
		}

		const TMFXNameId& libraryName;
		const XmlNodeRef& libraryParamsNode;

	private:
		std::vector<TMFXContainerPtr>& allContainers;
	};

public:

	void             LoadFromXml(SLoadingEnvironment& loadingEnvironment);
	TMFXContainerPtr FindEffectContainer(tukk effectName) const;
	void             GetMemoryUsage(IDrxSizer* pSizer) const;
	void             GetEffectNames(TEffectNames& nameList) const;

private:

	bool AddContainer(const TMFXNameId& effectName, TMFXContainerPtr pContainer);

private:

	TEffectContainersMap m_effectContainers;
};

#endif // _MFX_LIBRARY_H_
