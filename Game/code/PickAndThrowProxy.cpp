// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>

#include <drx3D/Game/PickAndThrowProxy.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameCVars.h>

DEFINE_SHARED_PARAMS_TYPE_INFO(CPickAndThrowProxy::SPnTProxyParams)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPickAndThrowProxy::SPnTProxyParams::SPnTProxyParams() : fRadius(0.5f), fHeight(0.5f), proxyShape(ePS_Capsule), vPosPivot(ZERO)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPickAndThrowProxyPtr CPickAndThrowProxy::Create(CPlayer* pPlayer, const IItemParamsNode* pParams)
{
	DRX_ASSERT(pPlayer && pParams);

	CPickAndThrowProxyPtr pNewInstance;
	if (pPlayer && pParams && !pPlayer->IsPlayer() && !pPlayer->IsPoolEntity() && g_pGameCVars->pl_pickAndThrow.useProxies)
	{
		const IItemParamsNode* pRootParams = pParams->GetChild("PickAndThrowProxyParams");
		if (pRootParams)
		{
			pNewInstance.reset(new CPickAndThrowProxy(*pPlayer));
			pNewInstance->ReadXmlData(pRootParams);
		}
	}

	return pNewInstance;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPickAndThrowProxy::CPickAndThrowProxy(CPlayer& player) : m_player(player), m_pPickNThrowProxy(NULL), m_pLastPlayerPhysics(NULL), m_bNeedsReloading(false)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CPickAndThrowProxy::~CPickAndThrowProxy()
{
	Unphysicalize();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPickAndThrowProxy::Physicalize()
{
	IEntity* pPlayerEntity = m_player.GetEntity();
	IPhysicalEntity* pPlayerPhysics = pPlayerEntity->GetPhysics();

	const bool bPhysicsChanged = (pPlayerPhysics != m_pLastPlayerPhysics);

	// Don't create it again if already exists and its associated physic entity is the same
	if ((m_pPickNThrowProxy && !bPhysicsChanged) || !g_pGameCVars->pl_pickAndThrow.useProxies)
		return;

	DRX_ASSERT(m_pParams != NULL);
	if (!m_pParams)
		return;

	if (bPhysicsChanged)
		Unphysicalize();

	pe_params_pos pp;
	pp.pos = pPlayerEntity->GetWorldPos();
	pp.iSimClass = SC_ACTIVE_RIGID;
	IPhysicalEntity* pPickNThrowProxy = gEnv->pPhysicalWorld->CreatePhysicalEntity(PE_ARTICULATED, &pp, pPlayerEntity, PHYS_FOREIGN_ID_ENTITY);

	phys_geometry *pGeom = NULL;
	switch(m_pParams->proxyShape)
	{
	case ePS_Capsule :
		{
			primitives::capsule prim;
			prim.axis.Set(0,0,1);
			prim.center.zero();
			prim.r = m_pParams->fRadius; 
			prim.hh = m_pParams->fHeight;
			IGeometry *pPrimGeom = gEnv->pPhysicalWorld->GetGeomUpr()->CreatePrimitive(primitives::capsule::type, &prim);
			pGeom = gEnv->pPhysicalWorld->GetGeomUpr()->RegisterGeometry(pPrimGeom, 0);

			pGeom->nRefCount = 0;
			pPrimGeom->Release();
		} break;
	case ePS_Sphere :
		{
			primitives::sphere prim;
			prim.center.zero();
			prim.r = m_pParams->fRadius; 
			IGeometry *pPrimGeom = gEnv->pPhysicalWorld->GetGeomUpr()->CreatePrimitive(primitives::sphere::type, &prim);
			pGeom = gEnv->pPhysicalWorld->GetGeomUpr()->RegisterGeometry(pPrimGeom, 0);

			pGeom->nRefCount = 0;
			pPrimGeom->Release();
		} break;
	case ePS_Cylinder :
		{
			primitives::cylinder prim;
			prim.axis.Set(0,0,1);
			prim.center.zero();
			prim.r = m_pParams->fRadius; 
			prim.hh = m_pParams->fHeight;
			IGeometry *pPrimGeom = gEnv->pPhysicalWorld->GetGeomUpr()->CreatePrimitive(primitives::cylinder::type, &prim);
			pGeom = gEnv->pPhysicalWorld->GetGeomUpr()->RegisterGeometry(pPrimGeom, 0);

			pGeom->nRefCount = 0;
			pPrimGeom->Release();
		} break;
	default:
		DRX_ASSERT_MESSAGE(false, "Invalid proxy shape?");
	}

	if (pGeom)
	{
		DRX_ASSERT(pPlayerEntity->GetPhysics() && (pPlayerEntity->GetPhysics()->GetType() == PE_LIVING));
		pe_params_articulated_body pab;
		pab.pHost = pPlayerPhysics;
		m_pLastPlayerPhysics = pPlayerPhysics;
		pab.posHostPivot = m_pParams->vPosPivot;
		pab.bGrounded = 1;
		pab.nJointsAlloc = 1;
		pPickNThrowProxy->SetParams(&pab);

		pe_articgeomparams gp;
		gp.pos.zero();
		gp.flags = CPickAndThrowProxy::geom_colltype_proxy;
		gp.flagsCollider = 0;
		gp.mass = 0.0f;
		pPickNThrowProxy->AddGeometry(pGeom, &gp);

		m_pPickNThrowProxy = pPickNThrowProxy;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPickAndThrowProxy::Unphysicalize()
{
	if (m_pPickNThrowProxy)
	{
		gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_pPickNThrowProxy);
		m_pPickNThrowProxy = NULL;
		m_pLastPlayerPhysics = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPickAndThrowProxy::OnReloadExtension()
{
	// Entity class could have changed, cache if we need reloading data
	ISharedParamsUpr* pSharedParamsUpr = gEnv->pGame->GetIGameFramework()->GetISharedParamsUpr();
	DRX_ASSERT(pSharedParamsUpr);

	// Query if params have changed
	DrxFixedStringT<64>	sharedParamsName = GetSharedParamsName();
	ISharedParamsConstPtr pSharedParams = pSharedParamsUpr->Get(sharedParamsName);

	m_bNeedsReloading = m_pParams != pSharedParams; 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CPickAndThrowProxy::ReadXmlData(const IItemParamsNode* pRootNode)
{
	ISharedParamsUpr* pSharedParamsUpr = gEnv->pGame->GetIGameFramework()->GetISharedParamsUpr();
	DRX_ASSERT(pSharedParamsUpr);

	// If we change the SharedParamsUpr to accept CRCs on its interface we could compute this once and store
	// the name's CRC32 instead of constructing it here each time this method is invoked (it shouldn't be invoked 
	// too often, though)
	DrxFixedStringT<64>	sharedParamsName = GetSharedParamsName();
	ISharedParamsConstPtr pSharedParams = pSharedParamsUpr->Get(sharedParamsName);
	if (pSharedParams)
	{
		m_pParams = CastSharedParamsPtr<SPnTProxyParams>(pSharedParams);
	}
	else
	{
		m_pParams.reset();

		SPnTProxyParams newParams;
		tukk szProxyShape = pRootNode->GetAttribute("proxyShape");
		if (szProxyShape)
		{
			if (strcmpi("capsule", szProxyShape) == 0)
			{
				newParams.proxyShape = ePS_Capsule;
			}
			else if (strcmpi("sphere", szProxyShape) == 0)
			{
				newParams.proxyShape = ePS_Sphere;
			}
			else if (strcmpi("cylinder", szProxyShape) == 0)
			{
				newParams.proxyShape = ePS_Cylinder;
			}
		}

		pRootNode->GetAttribute("proxyRadius", newParams.fRadius);
		pRootNode->GetAttribute("proxyHeight", newParams.fHeight);
		pRootNode->GetAttribute("proxyPivot", newParams.vPosPivot);

		m_pParams = CastSharedParamsPtr<SPnTProxyParams>(pSharedParamsUpr->Register(sharedParamsName, newParams));
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DrxFixedStringT<64>	CPickAndThrowProxy::GetSharedParamsName() const
{
	tukk szEntityClassName = m_player.GetEntityClassName();
	DrxFixedStringT<64>	sharedParamsName;
	sharedParamsName.Format("%s_%s", SPnTProxyParams::s_typeInfo.GetName(), szEntityClassName);

	return sharedParamsName;
}
