// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBinding.cpp
//  Version:     v1.00
//  Created:     9/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptBinding.h>

#include <drx3D/Script/ScriptBind_System.h>
#include <drx3D/Script/ScriptBind_Particle.h>
#include <drx3D/Script/ScriptBind_Sound.h>
#include <drx3D/Script/ScriptBind_Movie.h>
#include <drx3D/Script/ScriptBind_Script.h>
#include <drx3D/Script/ScriptBind_Physics.h>

#include <drx3D/Script/SurfaceTypes.h>

#include <drx3D/Script/IScriptSystem.h>

CScriptBindings::CScriptBindings()
{
	m_pScriptSurfaceTypes = 0;
}

CScriptBindings::~CScriptBindings()
{
	Done();
}

//////////////////////////////////////////////////////////////////////////
void CScriptBindings::Init(ISystem* pSystem, IScriptSystem* pSS)
{
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_System(pSS, pSystem)));
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_Particle(pSS, pSystem)));
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_Sound(pSS, pSystem)));
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_Movie(pSS, pSystem)));
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_Script(pSS, pSystem)));
	m_binds.push_back(std::unique_ptr<CScriptableBase>(new CScriptBind_Physics(pSS, pSystem)));

	//////////////////////////////////////////////////////////////////////////
	// Enumerate script surface types.
	//////////////////////////////////////////////////////////////////////////
	m_pScriptSurfaceTypes = new CScriptSurfaceTypesLoader;
}

//////////////////////////////////////////////////////////////////////////
void CScriptBindings::Done()
{
	if (m_pScriptSurfaceTypes)
		delete m_pScriptSurfaceTypes;
	m_pScriptSurfaceTypes = NULL;

	// Done script bindings.
	m_binds.clear();
}

//////////////////////////////////////////////////////////////////////////
void CScriptBindings::LoadScriptedSurfaceTypes(tukk sFolder, bool bReload)
{
	m_pScriptSurfaceTypes->LoadSurfaceTypes(sFolder, bReload);
}

//////////////////////////////////////////////////////////////////////////
void CScriptBindings::GetMemoryStatistics(IDrxSizer* pSizer) const
{
	//pSizer->AddObject(m_binds);
}
