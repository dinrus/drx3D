// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SceneElementPhysProxies.h"
#include "SceneElementTypes.h"
#include "ProxyGenerator/PhysicsProxies.h"

CSceneElementPhysProxies::CSceneElementPhysProxies(CScene* pScene, i32 id)
	: CSceneElementCommon(pScene, id)
	, m_pPhysProxies(nullptr)
{
}

void CSceneElementPhysProxies::SetPhysProxies(SPhysProxies* pPhysProxies)
{
	m_pPhysProxies = pPhysProxies;

	i32k nMeshes = m_pPhysProxies->nMeshes;
	string name;
	name.Format("Auto Proxies (%d source mesh%s)", nMeshes, nMeshes == 1 ? "" : "es");
	SetName(name);
}

SPhysProxies* CSceneElementPhysProxies::GetPhysProxies()
{
	return m_pPhysProxies;
}

void CSceneElementPhysProxies::Serialize(Serialization::IArchive& ar)
{
	struct SProxies
	{
		SPhysProxies* pProx;
		void          Serialize(Serialization::IArchive& ar)
		{
			pProx->Serialize(ar);
		}
	} proxies = { m_pPhysProxies };
	ar(proxies, "proxies", "Physics Proxies");
}

ESceneElementType CSceneElementPhysProxies::GetType() const
{
	return ESceneElementType::PhysProxy;
}

