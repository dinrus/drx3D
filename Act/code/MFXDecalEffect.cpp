// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MFXDecalEffect.cpp
//  Version:     v1.00
//  Created:     28/11/2006 by JohnN/AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Decal effect
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MFXDecalEffect.h>

CMFXDecalEffect::CMFXDecalEffect()
	: CMFXEffectBase(eMFXPF_Decal)
	, m_decalParams()
	, m_material(0)
{
}

CMFXDecalEffect::~CMFXDecalEffect()
{
	ReleaseMaterial();
}

void CMFXDecalEffect::LoadParamsFromXml(const XmlNodeRef& paramsNode)
{
	// Xml data format
	/*
	   <Decal minScale="..." maxscale="..." rotation="..." growTime="..." assembledecals="..." forceedge="..." lifetime="..." >
	   <Material>MaterialToUse</Material>
	   </Decal>
	 */

	XmlNodeRef material = paramsNode->findChild("Material");
	if (material)
	{
		m_decalParams.material = material->getContent();
	}

	m_decalParams.minscale = 1.f;
	m_decalParams.maxscale = 1.f;
	m_decalParams.rotation = -1.f;
	m_decalParams.growTime = 0.f;
	m_decalParams.assemble = false;
	m_decalParams.lifetime = 10.0f;
	m_decalParams.forceedge = false;

	paramsNode->getAttr("minscale", m_decalParams.minscale);
	paramsNode->getAttr("maxscale", m_decalParams.maxscale);

	paramsNode->getAttr("rotation", m_decalParams.rotation);
	m_decalParams.rotation = DEG2RAD(m_decalParams.rotation);

	paramsNode->getAttr("growTime", m_decalParams.growTime);
	paramsNode->getAttr("assembledecals", m_decalParams.assemble);
	paramsNode->getAttr("forceedge", m_decalParams.forceedge);
	paramsNode->getAttr("lifetime", m_decalParams.lifetime);
}

void CMFXDecalEffect::PreLoadAssets()
{
	if (m_decalParams.material.c_str())
	{
		// store as smart pointer
		m_material = gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial(
		  m_decalParams.material.c_str(), false);
	}
}

void CMFXDecalEffect::ReleasePreLoadAssets()
{
	ReleaseMaterial();
}

void CMFXDecalEffect::ReleaseMaterial()
{
	// Release material (smart pointer)
	m_material = 0;
}

void CMFXDecalEffect::Execute(const SMFXRunTimeEffectParams& params)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	const float angle = (params.angle != MFX_INVALID_ANGLE) ? params.angle : drx_random(0.f, gf_PI2);

	if (!params.trgRenderNode && !params.trg)
	{
		DinrusXDecalInfo terrainDecal;

		{
			// 2d terrain
			const float terrainHeight(gEnv->p3DEngine->GetTerrainElevation(params.pos.x, params.pos.y));
			const float terrainDelta(params.pos.z - terrainHeight);

			if (terrainDelta > 2.0f || terrainDelta < -0.5f)
				return;

			terrainDecal.vPos = Vec3(params.decalPos.x, params.decalPos.y, terrainHeight);
		}

		terrainDecal.vNormal = params.normal;
		terrainDecal.vHitDirection = params.dir[0].GetNormalized();
		terrainDecal.fLifeTime = m_decalParams.lifetime;
		terrainDecal.fGrowTime = m_decalParams.growTime;

		if (!m_decalParams.material.empty())
			drx_strcpy(terrainDecal.szMaterialName, m_decalParams.material.c_str());
		else
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "CMFXDecalEffect::Execute: Decal material name is not specified");

		terrainDecal.fSize = drx_random(m_decalParams.minscale, m_decalParams.maxscale);

		if (m_decalParams.rotation >= 0.f)
			terrainDecal.fAngle = m_decalParams.rotation;
		else
			terrainDecal.fAngle = angle;

		if (terrainDecal.fSize <= params.fDecalPlacementTestMaxSize)
			gEnv->p3DEngine->CreateDecal(terrainDecal);
	}
	else
	{
		DinrusXDecalInfo decal;

		IEntity* pEnt = gEnv->pEntitySystem->GetEntity(params.trg);
		IRenderNode* pRenderNode = NULL;
		if (pEnt)
		{
			IEntityRender* pIEntityRender = pEnt->GetRenderInterface();
			
				pRenderNode = pIEntityRender->GetRenderNode();
		}
		else
		{
			pRenderNode = params.trgRenderNode;
		}

		// filter out ropes
		if (pRenderNode && pRenderNode->GetRenderNodeType() == eERType_Rope)
			return;

		decal.ownerInfo.pRenderNode = pRenderNode;

		decal.vPos = params.pos;
		decal.vNormal = params.normal;
		decal.vHitDirection = params.dir[0].GetNormalized();
		decal.fLifeTime = m_decalParams.lifetime;
		decal.fGrowTime = m_decalParams.growTime;
		decal.bAssemble = m_decalParams.assemble;
		decal.bForceEdge = m_decalParams.forceedge;

		if (!m_decalParams.material.empty())
			drx_strcpy(decal.szMaterialName, m_decalParams.material.c_str());
		else
			DrxWarning(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, "CMFXDecalEffect::Execute: Decal material name is not specified");

		decal.fSize = drx_random(m_decalParams.minscale, m_decalParams.maxscale);
		if (m_decalParams.rotation >= 0.f)
			decal.fAngle = m_decalParams.rotation;
		else
			decal.fAngle = angle;

		if (decal.fSize <= params.fDecalPlacementTestMaxSize)
			gEnv->p3DEngine->CreateDecal(decal);
	}
}

void CMFXDecalEffect::GetResources(SMFXResourceList& resourceList) const
{
	SMFXDecalListNode* listNode = SMFXDecalListNode::Create();
	listNode->m_decalParams.material = m_decalParams.material.c_str();
	listNode->m_decalParams.minscale = m_decalParams.minscale;
	listNode->m_decalParams.maxscale = m_decalParams.maxscale;
	listNode->m_decalParams.rotation = m_decalParams.rotation;
	listNode->m_decalParams.assemble = m_decalParams.assemble;
	listNode->m_decalParams.forceedge = m_decalParams.forceedge;
	listNode->m_decalParams.lifetime = m_decalParams.lifetime;

	SMFXDecalListNode* next = resourceList.m_decalList;

	if (!next)
		resourceList.m_decalList = listNode;
	else
	{
		while (next->pNext)
			next = next->pNext;

		next->pNext = listNode;
	}
}

void CMFXDecalEffect::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_decalParams.material);
}
