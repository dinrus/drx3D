// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание: 
Holds a CMovementTransitions object for each entity class
-------------------------------------------------------------------------
История:
- 4:29:2010	19:48 : Created by Sven Van Soom
*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/MovementTransitions.h>
#include <drx3D/Game/MovementTransitionsSystem.h>

///////////////////////////////////////////////////////////////////////////////
CMovementTransitionsSystem::CMovementTransitionsSystem()
{
}


///////////////////////////////////////////////////////////////////////////////
CMovementTransitionsSystem::~CMovementTransitionsSystem()
{
	MovementTransitionsVector::iterator iEnd = m_entityTransitions.end();
	for(MovementTransitionsVector::iterator i = m_entityTransitions.begin(); i != iEnd; ++i)
	{
		delete (*i);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CMovementTransitionsSystem::Flush()
{
	MovementTransitionsVector::iterator iEnd = m_entityTransitions.end();
	for(MovementTransitionsVector::iterator i = m_entityTransitions.begin(); i != iEnd; ++i)
	{
		delete (*i);
	}
	stl::free_container(m_entityTransitions);
}

///////////////////////////////////////////////////////////////////////////////
void CMovementTransitionsSystem::GetMemoryUsage(IDrxSizer* s) const
{
	s->AddObject(this, sizeof(*this));

	MovementTransitionsVector::const_iterator iEnd = m_entityTransitions.end();
	for(MovementTransitionsVector::const_iterator i = m_entityTransitions.begin(); i != iEnd; ++i)
	{
		(*i)->GetMemoryUsage(s);
	}
}


///////////////////////////////////////////////////////////////////////////////
void CMovementTransitionsSystem::Reload() const
{
	MovementTransitionsVector::const_iterator iEnd = m_entityTransitions.end();
	for(MovementTransitionsVector::const_iterator i = m_entityTransitions.begin(); i != iEnd; ++i)
	{
		(*i)->Load();
	}
}


///////////////////////////////////////////////////////////////////////////////
CMovementTransitions*const CMovementTransitionsSystem::GetMovementTransitions(IEntity* pEntity)
{
	return GetMovementTransitions(pEntity->GetClass(), pEntity->GetScriptTable());
}


///////////////////////////////////////////////////////////////////////////////
CMovementTransitions*const CMovementTransitionsSystem::GetMovementTransitions(IEntityClass* pEntityClass, SmartScriptTable pEntityScript)
{
	// Check for useSpecialMovementTransitions property in LUA
	if(!pEntityScript)
		return NULL;

	SmartScriptTable pPropertiesTable;
	pEntityScript->GetValue("Properties", pPropertiesTable);
	if (!pPropertiesTable)
		return NULL;

	i32 useMovementTransitions = 0;
	if(!pPropertiesTable->GetValue("useSpecialMovementTransitions", useMovementTransitions) || !useMovementTransitions)
		return NULL;

	// Look for cached movement transitions
	MovementTransitionsVector::iterator iEnd = m_entityTransitions.end();
	for(MovementTransitionsVector::iterator i = m_entityTransitions.begin(); i != iEnd; ++i)
	{
		if ((*i)->GetEntityClass() == pEntityClass)
		{
			return (*i);
		}
	}

	// Create new movement transition
	CMovementTransitions* pTransitions = new CMovementTransitions(pEntityClass);
	DRX_ASSERT(pTransitions);
	if (pTransitions)
		m_entityTransitions.push_back(pTransitions);
	return pTransitions;
}

