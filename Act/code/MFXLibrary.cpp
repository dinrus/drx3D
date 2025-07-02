// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MFXLibrary.h>

namespace MaterialEffectsUtils
{
CMFXContainer* CreateContainer()
{
	if (gEnv->IsDedicated())
	{
		return new CMFXDummyContainer();
	}
	else
	{
		return new CMFXContainer();
	}
}
}

void CMFXLibrary::LoadFromXml(SLoadingEnvironment& loadingEnvironment)
{
	DrxLogAlways("[MFX] Loading FXLib '%s' ...", loadingEnvironment.libraryName.c_str());

	INDENT_LOG_DURING_SCOPE();

	for (i32 i = 0; i < loadingEnvironment.libraryParamsNode->getChildCount(); ++i)
	{
		XmlNodeRef currentEffectNode = loadingEnvironment.libraryParamsNode->getChild(i);
		if (!currentEffectNode)
			continue;

		TMFXContainerPtr pContainer = MaterialEffectsUtils::CreateContainer();
		pContainer->BuildFromXML(currentEffectNode);

		const TMFXNameId& effectName = pContainer->GetParams().name;
		const bool effectAdded = AddContainer(effectName, pContainer);
		if (effectAdded)
		{
			loadingEnvironment.AddLibraryContainer(loadingEnvironment.libraryName, pContainer);
		}
		else
		{
			GameWarning("[MFX] Effect (at line %d) '%s:%s' already present, skipping redefinition!", currentEffectNode->getLine(), loadingEnvironment.libraryName.c_str(), effectName.c_str());
		}
	}
}

bool CMFXLibrary::AddContainer(const TMFXNameId& effectName, TMFXContainerPtr pContainer)
{
	TEffectContainersMap::const_iterator it = m_effectContainers.find(effectName);
	if (it != m_effectContainers.end())
		return false;

	m_effectContainers.insert(TEffectContainersMap::value_type(effectName, pContainer));
	return true;
}

TMFXContainerPtr CMFXLibrary::FindEffectContainer(tukk effectName) const
{
	TEffectContainersMap::const_iterator it = m_effectContainers.find(CONST_TEMP_STRING(effectName));

	return (it != m_effectContainers.end()) ? it->second : TMFXContainerPtr(NULL);
}

void CMFXLibrary::GetEffectNames(TEffectNames& nameList) const
{
	for (const auto& curEffect : m_effectContainers)
	{
		nameList.push_back(curEffect.first.c_str());
	}
}

void CMFXLibrary::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_effectContainers);
}
