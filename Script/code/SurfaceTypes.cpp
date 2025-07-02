// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SurfaceTypes.cpp
//  Version:     v1.00
//  Created:     30/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/SurfaceTypes.h>

#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Phys/IPhysics.h>

static XmlNodeRef m_root = 0;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
class CScriptSurfaceType : public ISurfaceType
{
public:
	CScriptSurfaceType(CScriptSurfaceTypesLoader* pUpr, tukk sName, tukk sFilename, i32 nFlags);
	~CScriptSurfaceType();
	virtual void                        Release()                                                           { delete this; }
	virtual u16                      GetId() const                                                       { return m_nId; }
	virtual tukk                 GetName() const                                                     { return m_name; }
	virtual tukk                 GetType() const                                                     { return ""; };
	virtual i32                         GetFlags() const                                                    { return m_nFlags; };
	virtual void                        Execute(SSurfaceTypeExecuteParams& params);
	virtual struct IScriptTable*        GetScriptTable()                                                    { return m_pScriptTable; };
	virtual bool                        Load(i32 nId);
	virtual i32                         GetBreakability() const                                             { return m_iBreakability; }
	virtual i32                         GetHitpoints() const                                                { return m_nHitpoints; }
	virtual float                       GetBreakEnergy() const                                              { return (float)m_breakEnergy; }
	virtual void                        Get(SSurfaceTypeAIParams& aiParams)                                 {};
	virtual const SSurfaceTypeAIParams* GetAIParams()                                                       { return 0; };
	virtual const SPhysicalParams& GetPhyscalParams()                                                  { return m_physParams; };
	virtual SBreakable2DParams*    GetBreakable2DParams()                                              { return 0; };
	virtual SBreakageParticles*    GetBreakageParticles(tukk sType, bool bLookInDefault = true) { return 0; };

private:
	CScriptSurfaceTypesLoader* m_pUpr;
	string                     m_name;
	string                     m_script;
	i32                        m_nId;
	i32                        m_nFlags;
	i32                        m_iBreakability;
	i32                        m_nHitpoints;
	i32                        m_breakEnergy;
	SPhysicalParams            m_physParams;
	SSurfaceTypeAIParams       m_aiParams;
	SmartScriptTable           m_pScriptTable;
};

//////////////////////////////////////////////////////////////////////////
CScriptSurfaceType::CScriptSurfaceType(CScriptSurfaceTypesLoader* pUpr, tukk sName, tukk sFilename, i32 nFlags)
	: m_pUpr(pUpr)
	, m_name(sName)
	, m_script(sFilename)
	, m_nId(-1)
	, m_nFlags(nFlags)
	, m_iBreakability(0)
	, m_nHitpoints(0)
	, m_breakEnergy(0)
{
}

//////////////////////////////////////////////////////////////////////////
CScriptSurfaceType::~CScriptSurfaceType()
{
	m_pUpr->UnregisterSurfaceType(this);
}

//////////////////////////////////////////////////////////////////////////
bool CScriptSurfaceType::Load(i32 nId)
{
	m_nId = nId;
	IScriptSystem* pScriptSystem = gEnv->pScriptSystem;

	SmartScriptTable mtlTable;

	if (!pScriptSystem->GetGlobalValue("Materials", mtlTable))
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	if (!pScriptSystem->ExecuteFile(m_script, true))
	{
		GetISystem()->Warning(
		  VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING,
		  VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_SCRIPT,
		  m_script.c_str(),
		  "'%s' failed to load surface type definition script", m_name.c_str());
		return false;
	}

	if (!mtlTable->GetValue(m_name, m_pScriptTable))
		return false;

	XmlNodeRef matNode = m_root->newChild("SurfaceType");
	matNode->setAttr("name", m_name);

	// Load physics params.
	SmartScriptTable pPhysicsTable, props;
	float fBouncyness = 0.0f;
	float fFriction = 1.0f;
	i32 iPiercingResistence = sf_max_pierceable;    // physics traces range 0-15
	i32 imatBreakable = -1, bManuallyBreakable = 0;
	m_iBreakability = 0;
	m_nHitpoints = 0;
	m_breakEnergy = 0;
	if (m_pScriptTable->GetValue("physics", pPhysicsTable))
	{
		pPhysicsTable->GetValue("friction", fFriction);
		pPhysicsTable->GetValue("bouncyness", fBouncyness);
		pPhysicsTable->GetValue("breakable_id", imatBreakable);
		if (pPhysicsTable->GetValue("pierceability", iPiercingResistence))
		{
			if (iPiercingResistence > sf_max_pierceable)
				iPiercingResistence = sf_max_pierceable;
		}
		i32 nBreakable2d = 0;
		i32 bNoCollide = 0;
		pPhysicsTable->GetValue("no_collide", bNoCollide);
		if (pPhysicsTable->GetValue("break_energy", m_breakEnergy))
		{
			bManuallyBreakable = sf_manually_breakable;
			m_iBreakability = 2;
			pPhysicsTable->GetValue("hit_points", m_nHitpoints);
		}
		else if (m_pScriptTable->GetValue("breakable_2d", props))
		{
			nBreakable2d = 1;
			bManuallyBreakable = sf_manually_breakable;
			m_iBreakability = 1;
			props->GetValue("break_energy", m_breakEnergy);
			props->GetValue("hit_points", m_nHitpoints);
		}

		m_nFlags &= ~SURFACE_TYPE_NO_COLLIDE;
		if (bNoCollide)
			m_nFlags |= SURFACE_TYPE_NO_COLLIDE;

		XmlNodeRef physNode = matNode->newChild("Physics");
		physNode->setAttr("friction", fFriction);
		physNode->setAttr("elasticity", fBouncyness);
		physNode->setAttr("breakable_id", imatBreakable);
		physNode->setAttr("pierceability", iPiercingResistence);
		physNode->setAttr("no_collide", bNoCollide);
		physNode->setAttr("break_energy", m_breakEnergy);
		physNode->setAttr("hit_points", m_nHitpoints);
		physNode->setAttr("breakable_2d", nBreakable2d);
	}

	SmartScriptTable pAITable;
	if (m_pScriptTable->GetValue("AI", pAITable))
	{
		XmlNodeRef aiNode = matNode->newChild("AI");
		float fImpactRadius = 1;
		float fFootStepRadius = 1;
		float proneMult = 1;
		float crouchMult = 1;
		float movingMult = 1;
		pAITable->GetValue("fImpactRadius", fImpactRadius);
		pAITable->GetValue("fFootStepRadius", fFootStepRadius);
		pAITable->GetValue("proneMult", proneMult);
		pAITable->GetValue("crouchMult", crouchMult);
		pAITable->GetValue("movingMult", movingMult);

		aiNode->setAttr("fImpactRadius", fImpactRadius);
		aiNode->setAttr("fFootStepRadius", fFootStepRadius);
		aiNode->setAttr("proneMult", proneMult);
		aiNode->setAttr("crouchMult", crouchMult);
		aiNode->setAttr("movingMult", movingMult);
	}
	gEnv->pPhysicalWorld->SetSurfaceParameters(m_nId, fBouncyness, fFriction,
	                                           (u32)(sf_pierceability(iPiercingResistence) | sf_matbreakable(imatBreakable) | bManuallyBreakable));

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptSurfaceType::Execute(SSurfaceTypeExecuteParams& params)
{
}

//////////////////////////////////////////////////////////////////////////
CScriptSurfaceTypesLoader::CScriptSurfaceTypesLoader()
{
}

//////////////////////////////////////////////////////////////////////////
CScriptSurfaceTypesLoader::~CScriptSurfaceTypesLoader()
{
	UnloadSurfaceTypes();
}

//////////////////////////////////////////////////////////////////////////
void CScriptSurfaceTypesLoader::ReloadSurfaceTypes()
{
	for (u32 i = 0; i < m_folders.size(); i++)
	{
		LoadSurfaceTypes(m_folders[i], true);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScriptSurfaceTypesLoader::LoadSurfaceTypes(tukk sFolder, bool bReload)
{
	{
		if (!gEnv->p3DEngine)
			return false;

		I3DEngine* pEngine = gEnv->p3DEngine;
		ISurfaceTypeEnumerator* pEnum = pEngine->GetMaterialUpr()->GetSurfaceTypeUpr()->GetEnumerator();
		if (pEnum)
		{
			for (ISurfaceType* pSurfaceType = pEnum->GetFirst(); pSurfaceType; pSurfaceType = pEnum->GetNext())
			{
				SmartScriptTable mtlTable(gEnv->pScriptSystem);
				gEnv->pScriptSystem->SetGlobalValue(pSurfaceType->GetName(), mtlTable);

				SmartScriptTable aiTable(gEnv->pScriptSystem);
				mtlTable->SetValue("AI", aiTable);
				aiTable->SetValue("fImpactRadius", 5.0f);
				aiTable->SetValue("fFootStepRadius", 15.0f);
				aiTable->SetValue("proneMult", 0.2f);
				aiTable->SetValue("crouchMult", 0.5f);
				aiTable->SetValue("movingMult", 2.5f);
			}

			pEnum->Release();
		}
	}

	return true; // Do not load surface types from script anymore.

	m_root = GetISystem()->CreateXmlNode("SurfaceTypes");

	IScriptSystem* pScriptSystem = gEnv->pScriptSystem;
	//////////////////////////////////////////////////////////////////////////
	// Make sure Materials table exist.
	//////////////////////////////////////////////////////////////////////////
	SmartScriptTable mtlTable;

	if (!pScriptSystem->GetGlobalValue("Materials", mtlTable) || bReload)
	{
		mtlTable = pScriptSystem->CreateTable();
		pScriptSystem->SetGlobalValue("Materials", mtlTable);
	}

	IDrxPak* pIPak = gEnv->pDrxPak;

	ISurfaceTypeUpr* pSurfaceUpr = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceTypeUpr();

	if (!bReload)
		stl::push_back_unique(m_folders, sFolder);

	string searchFolder = string(sFolder) + "/";
	;
	string searchFilter = searchFolder + "mat_*.lua";

	gEnv->pScriptSystem->ExecuteFile(searchFolder + "common.lua", false, bReload);

	_finddata_t fd;
	intptr_t fhandle;
	fhandle = pIPak->FindFirst(searchFilter, &fd);
	if (fhandle != -1)
	{
		do
		{
			// Skip back folders.
			if (fd.attrib & _A_SUBDIR) // skip if directory.
				continue;

			char name[_MAX_PATH];
			_splitpath(fd.name, NULL, NULL, name, NULL);

			if (strlen(name) == 0)
				continue;

			if (bReload)
			{
				ISurfaceType* pSurfaceType = pSurfaceUpr->GetSurfaceTypeByName(name);
				if (pSurfaceType)
				{
					pSurfaceType->Load(pSurfaceType->GetId());
					continue;
				}
			}

			ISurfaceType* pSurfaceType = new CScriptSurfaceType(this, name, searchFolder + fd.name, 0);
			if (pSurfaceUpr->RegisterSurfaceType(pSurfaceType))
				m_surfaceTypes.push_back(pSurfaceType);
			else
				pSurfaceType->Release();
		}
		while (pIPak->FindNext(fhandle, &fd) == 0);
		pIPak->FindClose(fhandle);
	}

	if (m_root)
	{
		m_root->saveToFile("SurfaceTypes.xml");
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptSurfaceTypesLoader::UnloadSurfaceTypes()
{
	ISurfaceTypeUpr* pSurfaceUpr = NULL;
	I3DEngine* pEngine = gEnv->p3DEngine;
	if (pEngine)
	{
		pSurfaceUpr = pEngine->GetMaterialUpr()->GetSurfaceTypeUpr();
	}
	for (u32 i = 0; i < m_surfaceTypes.size(); i++)
	{
		if (pSurfaceUpr)
			pSurfaceUpr->UnregisterSurfaceType(m_surfaceTypes[i]);
		m_surfaceTypes[i]->Release();
	}
	m_surfaceTypes.clear();
}

//////////////////////////////////////////////////////////////////////////
void CScriptSurfaceTypesLoader::UnregisterSurfaceType(ISurfaceType* sfType)
{
	stl::find_and_erase(m_surfaceTypes, sfType);
}
