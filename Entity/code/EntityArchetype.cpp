// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/EntityClass.h>
#include <drx3D/Entity/EntityScript.h>
#include <drx3D/Entity/EntityArchetype.h>
#include <drx3D/Entity/ScriptProperties.h>
#include <drx3D/CoreX/String/Path.h>
#include <drx3D/Entity/EntitySystem.h>

#define ENTITY_ARCHETYPES_LIBS_PATH "/Libs/EntityArchetypes/"

//////////////////////////////////////////////////////////////////////////
CEntityArchetype::CEntityArchetype(IEntityClass* pClass)
{
	assert(pClass);
	assert(pClass->GetScriptFile());
	m_pClass = pClass;

	// Try to load the script if it is not yet valid.
	if (CEntityScript* pScript = static_cast<CEntityScript*>(m_pClass->GetIEntityScript()))
	{
		pScript->LoadScript();

		if (pScript->GetPropertiesTable())
		{
			m_pProperties.Create(gEnv->pScriptSystem);
			m_pProperties->Clone(pScript->GetPropertiesTable(), true, true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetype::LoadFromXML(XmlNodeRef& propertiesNode, XmlNodeRef& objectVarsNode)
{
	// Lua specific behavior
	if (m_pClass->GetScriptTable() != nullptr)
	{
		m_ObjectVars = objectVarsNode->clone();

		if (!m_pProperties)
			return;

		CScriptProperties scriptProps;
		// Initialize properties.
		scriptProps.Assign(propertiesNode, m_pProperties);

		// Here we add the additional archetype properties, which are not
		// loaded before, as they are not supposed to have equivalent script
		// properties.
		// Still, we inject those properties into the script environment so
		// they will be available during entity creation.
		XmlNodeRef rAdditionalProperties = propertiesNode->findChild("AdditionalArchetypeProperties");
		if (rAdditionalProperties)
		{
			i32k nNumberOfAttributes(rAdditionalProperties->getNumAttributes());
			i32 nCurrentAttribute(0);

			tukk pszKey(NULL);
			tukk pszValue(NULL);

			tukk pszCurrentValue(NULL);

			for (/*nCurrentAttribute=0*/; nCurrentAttribute < nNumberOfAttributes; ++nCurrentAttribute)
			{
				rAdditionalProperties->getAttributeByIndex(nCurrentAttribute, &pszKey, &pszValue);
				if (!m_pProperties->GetValue(pszKey, pszCurrentValue))
				{
					m_pProperties->SetValue(pszKey, pszValue);
				}
				else
				{
					// In editor this will happen, but shouldn't generate a warning.
					if (!gEnv->IsEditor())
					{
						DrxLog("[EntitySystem] CEntityArchetype::LoadFromXML: attribute %s couldn't be injected into the script, as it was already present there.", pszKey);
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CEntityArchetypeUpr::CEntityArchetypeUpr()
	: m_pEntityArchetypeUprExtension(nullptr)
{
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeUpr::CreateArchetype(IEntityClass* pClass, tukk sArchetype)
{
	CEntityArchetype* pArchetype = stl::find_in_map(m_nameToArchetypeMap, sArchetype, NULL);
	if (pArchetype)
		return pArchetype;
	pArchetype = new CEntityArchetype(static_cast<CEntityClass*>(pClass));
	pArchetype->SetName(sArchetype);
	m_nameToArchetypeMap[pArchetype->GetName()] = pArchetype;

	if (m_pEntityArchetypeUprExtension)
	{
		m_pEntityArchetypeUprExtension->OnArchetypeAdded(*pArchetype);
	}

	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeUpr::FindArchetype(tukk sArchetype)
{
	CEntityArchetype* pArchetype = stl::find_in_map(m_nameToArchetypeMap, sArchetype, NULL);
	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetype* CEntityArchetypeUpr::LoadArchetype(tukk sArchetype)
{
	IEntityArchetype* pArchetype = FindArchetype(sArchetype);
	if (pArchetype)
		return pArchetype;

	const string& sLibName = GetLibraryFromName(sArchetype);

	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_ArchetypeLib, 0, sLibName.c_str());

	// If archetype is not found try to load the library first.
	if (LoadLibrary(sLibName))
	{
		pArchetype = FindArchetype(sArchetype);
	}

	return pArchetype;
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetypeUpr::UnloadArchetype(tukk sArchetype)
{
	ArchetypesNameMap::iterator it = m_nameToArchetypeMap.find(sArchetype);
	if (it != m_nameToArchetypeMap.end())
	{
		if (m_pEntityArchetypeUprExtension)
		{
			m_pEntityArchetypeUprExtension->OnArchetypeRemoved(*it->second);
		}

		m_nameToArchetypeMap.erase(it);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetypeUpr::Reset()
{
	MEMSTAT_LABEL_SCOPED("CEntityArchetypeUpr::Reset");

	if (m_pEntityArchetypeUprExtension)
	{
		m_pEntityArchetypeUprExtension->OnAllArchetypesRemoved();
	}

	m_nameToArchetypeMap.clear();
	DynArray<string>().swap(m_loadedLibs);
}

//////////////////////////////////////////////////////////////////////////
string CEntityArchetypeUpr::GetLibraryFromName(const string& sArchetypeName)
{
	string libname = sArchetypeName.SpanExcluding(".");
	return libname;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityArchetypeUpr::LoadLibrary(const string& library)
{
	if (stl::find(m_loadedLibs, library))
		return true;

	string filename;
	if (library == "Level")
	{
		filename = gEnv->p3DEngine->GetLevelFilePath("LevelPrototypes.xml");
	}
	else
	{
		filename = PathUtil::Make(PathUtil::GetGameFolder() + ENTITY_ARCHETYPES_LIBS_PATH, library, "xml");
	}

	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(filename);
	if (!rootNode)
		return false;

	IEntityClassRegistry* pClassRegistry = GetIEntitySystem()->GetClassRegistry();
	// Load all archetypes from library.

	for (i32 i = 0; i < rootNode->getChildCount(); i++)
	{
		XmlNodeRef node = rootNode->getChild(i);
		if (node->isTag("EntityPrototype"))
		{
			tukk name = node->getAttr("Name");
			tukk className = node->getAttr("Class");

			IEntityClass* pClass = pClassRegistry->FindClass(className);
			if (!pClass)
			{
				// No such entity class.
				EntityWarning("EntityArchetype %s references unknown entity class %s", name, className);
				continue;
			}

			string fullname = library + "." + name;
			IEntityArchetype* pArchetype = CreateArchetype(pClass, fullname);
			if (!pArchetype)
				continue;

			// Load properties.
			XmlNodeRef props = node->findChild("Properties");
			XmlNodeRef objVars = node->findChild("ObjectVars");
			if (props)
			{
				pArchetype->LoadFromXML(props, objVars);
			}

			if (m_pEntityArchetypeUprExtension)
			{
				m_pEntityArchetypeUprExtension->LoadFromXML(*pArchetype, node);
			}
		}
	}

	// Add this library to the list of loaded archetype libs.
	m_loadedLibs.push_back(library);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntityArchetypeUpr::SetEntityArchetypeUprExtension(IEntityArchetypeUprExtension* pEntityArchetypeUprExtension)
{
	m_pEntityArchetypeUprExtension = pEntityArchetypeUprExtension;
}

//////////////////////////////////////////////////////////////////////////
IEntityArchetypeUprExtension* CEntityArchetypeUpr::GetEntityArchetypeUprExtension() const
{
	return m_pEntityArchetypeUprExtension;
}
