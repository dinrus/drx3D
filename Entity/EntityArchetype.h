// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Script/IScriptSystem.h>

class CEntityClass;

//////////////////////////////////////////////////////////////////////////
class CEntityArchetype : public IEntityArchetype, public _i_reference_target_t
{
public:
	explicit CEntityArchetype(IEntityClass* pClass);

	//////////////////////////////////////////////////////////////////////////
	// IEntityArchetype
	//////////////////////////////////////////////////////////////////////////
	virtual IEntityClass* GetClass() const override { return m_pClass; }
	virtual tukk   GetName() const override  { return m_name.c_str(); }
	virtual IScriptTable* GetProperties() override  { return m_pProperties; }
	virtual XmlNodeRef    GetObjectVars() override  { return m_ObjectVars; }
	virtual void          LoadFromXML(XmlNodeRef& propertiesNode, XmlNodeRef& objectVarsNode) override;
	//////////////////////////////////////////////////////////////////////////

	void SetName(const string& sName) { m_name = sName; };

private:
	string           m_name;
	SmartScriptTable m_pProperties;
	XmlNodeRef       m_ObjectVars;
	IEntityClass*    m_pClass;
};

//////////////////////////////////////////////////////////////////////////
// Manages collection of the entity archetypes.
//////////////////////////////////////////////////////////////////////////
class CEntityArchetypeUpr
{
public:
	CEntityArchetypeUpr();

	IEntityArchetype*                 CreateArchetype(IEntityClass* pClass, tukk sArchetype);
	IEntityArchetype*                 FindArchetype(tukk sArchetype);
	IEntityArchetype*                 LoadArchetype(tukk sArchetype);
	void                              UnloadArchetype(tukk sArchetype);

	void                              Reset();

	void                              SetEntityArchetypeUprExtension(IEntityArchetypeUprExtension* pEntityArchetypeUprExtension);
	IEntityArchetypeUprExtension* GetEntityArchetypeUprExtension() const;

private:
	bool   LoadLibrary(const string& library);
	string GetLibraryFromName(const string& sArchetypeName);

	typedef std::map<tukk , _smart_ptr<CEntityArchetype>, stl::less_stricmp<tukk >> ArchetypesNameMap;
	ArchetypesNameMap                 m_nameToArchetypeMap;

	DynArray<string>                  m_loadedLibs;

	IEntityArchetypeUprExtension* m_pEntityArchetypeUprExtension;
};
