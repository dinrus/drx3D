// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 08:12:2010		Created by Ben Parbury
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/WeaponAlias.h>

//---------------------------------------
CWeaponAlias::CWeaponAlias()
{
	Reset();
}

//---------------------------------------
CWeaponAlias::~CWeaponAlias()
{
	m_aliases.clear();
}

//---------------------------------------
void CWeaponAlias::Reset()
{
	m_aliases.clear();
	m_aliases.reserve(16);
}

//---------------------------------------
void CWeaponAlias::AddAlias(tukk pParentName, tukk pChildName)
{
	SWeaponAlias alias(pParentName, pChildName);
	m_aliases.push_back(alias);
}

//---------------------------------------
const IEntityClass* CWeaponAlias::GetParentClass(const IEntityClass* pClass) const
{
	const size_t aliasCount = m_aliases.size();
	for(size_t i = 0; i < aliasCount; i++)
	{
		const SWeaponAlias& pAlias = m_aliases[i];
		if(pClass == pAlias.m_pClass)
		{
			return pAlias.m_pParentClass;
		}
	}

	return NULL;
}

//---------------------------------------
const IEntityClass* CWeaponAlias::GetParentClass( tukk pClassName ) const
{
	const IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry();
	IEntityClass* pClass = pClassRegistry->FindClass(pClassName);
	if(pClass)
	{
		return GetParentClass(pClass);
	}

	return NULL;
}

//---------------------------------------
void CWeaponAlias::UpdateClass(IEntityClass** ppClass) const
{
	const IEntityClass* pClass = *ppClass;
	const size_t aliasCount = m_aliases.size();
	for(size_t i = 0; i < aliasCount; i++)
	{
		const SWeaponAlias& pAlias = m_aliases[i];
		if(pClass == pAlias.m_pClass)
		{
			*ppClass = pAlias.m_pParentClass;
			return;
		}
	}
}

//---------------------------------------
bool CWeaponAlias::IsAlias(tukk pAliasName) const
{
	const size_t aliasCount = m_aliases.size();
	for(size_t i = 0; i < aliasCount; i++)
	{
		const SWeaponAlias& pAlias = m_aliases[i];
		if(strcmpi(pAlias.m_pClass->GetName(), pAliasName) == 0)
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////
CWeaponAlias::SWeaponAlias::SWeaponAlias(tukk pParentName, tukk pName)
{
	const IEntityClassRegistry* pClassRegistry = gEnv->pEntitySystem->GetClassRegistry();

	m_pParentClass = pClassRegistry->FindClass(pParentName);
	DRX_ASSERT(m_pParentClass);

	m_pClass = pClassRegistry->FindClass(pName);
	DRX_ASSERT(m_pClass);
}
