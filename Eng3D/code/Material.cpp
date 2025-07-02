// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Material.cpp
//  Version:     v1.00
//  Created:     3/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/MatMan.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Eng3D/VisAreas.h>

DEFINE_INTRUSIVE_LINKED_LIST(CMatInfo)

//////////////////////////////////////////////////////////////////////////
void CMaterialLayer::SetShaderItem(const IMaterial* pParentMtl, const SShaderItem& pShaderItem)
{
	assert(pParentMtl && "CMaterialLayer::SetShaderItem invalid material");

	if (pShaderItem.m_pShader)
	{
		pShaderItem.m_pShader->AddRef();
	}

	if (pShaderItem.m_pShaderResources)
	{
		pShaderItem.m_pShaderResources->AddRef();
		CMatInfo* pParentMatInfo = (CMatInfo*)(const_cast<IMaterial*>(pParentMtl));
		pShaderItem.m_pShaderResources->SetMaterialName(pParentMatInfo->m_sUniqueMaterialName);
	}

	SAFE_RELEASE(m_pShaderItem.m_pShader);
	SAFE_RELEASE(m_pShaderItem.m_pShaderResources);

	m_pShaderItem = pShaderItem;
	gEnv->pRenderer->UpdateShaderItem(&m_pShaderItem, NULL);

}
//////////////////////////////////////////////////////////////////////////
void CMaterialLayer::GetMemoryUsage(IDrxSizer* pSizer)
{
	SIZER_COMPONENT_NAME(pSizer, "MaterialLayer");
	pSizer->AddObject(this, sizeof(*this));
}

//////////////////////////////////////////////////////////////////////////
size_t CMaterialLayer::GetResourceMemoryUsage(IDrxSizer* pSizer)
{
	size_t nResourceMemory(0);
	// Pushing some context to use the Macro.
	{
		SIZER_COMPONENT_NAME(pSizer, "Textures");

		IRenderShaderResources* piResources(m_pShaderItem.m_pShaderResources);
		if (piResources)
		{
			//////////////////////////////////////////////////////////////////////////
			// Texture
			for (size_t nCount = 0; nCount < EFTT_MAX; ++nCount)
			{
				SEfResTexture* piTextureResource(piResources->GetTexture(nCount));
				if (piTextureResource)
				{
					ITexture* piTexture = piTextureResource->m_Sampler.m_pITex;
					if (piTexture)
					{
						SIZER_COMPONENT_NAME(pSizer, "MemoryTexture");
						size_t nCurrentResourceMemoryUsage = piTexture->GetDataSize();
						nResourceMemory += nCurrentResourceMemoryUsage;
						pSizer->AddObject(piTexture, nCurrentResourceMemoryUsage);

						IResourceCollector* pColl = pSizer->GetResourceCollector();
						if (pColl)
						{
							pColl->AddResource(piTexture->GetName(), nCurrentResourceMemoryUsage);
						}
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////
		}
	}
	return nResourceMemory;
}

//////////////////////////////////////////////////////////////////////////
CMatInfo::CMatInfo()
	: m_bDeleted(false)
	, m_bDeletePending(false)
{
	m_nRefCount = 0;
	m_Flags = 0;
	m_nModificationId = 0;

	m_nSurfaceTypeId = 0;

	m_pMaterialLayers = 0;

	m_ucDefautMappingAxis = 0;
	m_fDefautMappingScale = 1.f;

#ifdef SUPPORT_MATERIAL_EDITING
	m_pUserData = NULL;
#endif

	m_pActiveLayer = NULL;

	ZeroStruct(m_streamZoneInfo);

#ifdef TRACE_MATERIAL_LEAKS
	m_sLoadingCallstack = GetSystem()->GetLoadingProfilerCallstack();
#endif

#if defined(ENABLE_CONSOLE_MTL_VIZ)
	m_pConsoleMtl = 0;
#endif
}

//////////////////////////////////////////////////////////////////////////
CMatInfo::~CMatInfo()
{
	ShutDown();
}

void CMatInfo::AddRef()
{
	DRX_ASSERT(!m_bDeletePending);
	DrxInterlockedIncrement(&m_nRefCount);
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::Release()
{
	if (DrxInterlockedDecrement(&m_nRefCount) == 0)
	{
		if (GetMatMan())
		{
			((CMatMan*)GetMatMan())->DelayedDelete(this);
		}
		else
		{
			delete this;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool CMatInfo::IsValid() const
{
	return !m_bDeletePending && !m_bDeleted;
}

void CMatInfo::ShutDown()
{
	if (!m_bDeleted)
	{
		SAFE_DELETE(m_pMaterialLayers);

		if (m_shaderItem.m_pShaderResources)
		{
			CCamera* pC = m_shaderItem.m_pShaderResources->GetCamera();
			if (pC)
				delete pC;
		}
		SAFE_RELEASE(m_shaderItem.m_pShader);
		SAFE_RELEASE(m_shaderItem.m_pShaderResources);

		m_subMtls.clear();
	}

	m_bDeleted = true;
}

//////////////////////////////////////////////////////////////////////////
IMaterialHelpers& CMatInfo::GetMaterialHelpers()
{
	return GetMatMan()->s_materialHelpers;
}

IMaterialUpr* CMatInfo::GetMaterialUpr()
{
	return GetMatMan();
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetName(tukk sName)
{
	m_sMaterialName = sName;
	m_sUniqueMaterialName = m_sMaterialName;
	if (m_shaderItem.m_pShaderResources)
		m_shaderItem.m_pShaderResources->SetMaterialName(m_sUniqueMaterialName); // Only for correct warning message purposes.
	if (m_Flags & MTL_FLAG_MULTI_SUBMTL)
	{
		for (i32 i = 0, num = m_subMtls.size(); i < num; i++)
		{
			if (m_subMtls[i] && (m_subMtls[i]->m_Flags & MTL_FLAG_PURE_CHILD))
			{
				m_subMtls[i]->m_sUniqueMaterialName = m_sMaterialName;
				if (m_subMtls[i]->m_shaderItem.m_pShaderResources)
					m_subMtls[i]->m_shaderItem.m_pShaderResources->SetMaterialName(m_sUniqueMaterialName);
			}

			if (m_subMtls[i] && strstr(m_subMtls[i]->m_sUniqueMaterialName, MTL_SPECIAL_NAME_RAYCAST_PROXY) != 0)
			{
				m_subMtls[i]->m_Flags |= MTL_FLAG_RAYCAST_PROXY;
				m_subMtls[i]->m_Flags |= MTL_FLAG_NODRAW;
			}
		}
	}

	if (strstr(sName, MTL_SPECIAL_NAME_COLLISION_PROXY) != 0 || strstr(sName, MTL_SPECIAL_NAME_COLLISION_PROXY_VEHICLE) != 0)
	{
		m_Flags |= MTL_FLAG_COLLISION_PROXY;
	}
	else if (strstr(sName, MTL_SPECIAL_NAME_RAYCAST_PROXY) != 0)
	{
		m_Flags |= MTL_FLAG_RAYCAST_PROXY;
		m_Flags |= MTL_FLAG_NODRAW;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CMatInfo::IsDefault() const
{
	return this == GetMatMan()->GetDefaultMaterial();
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::UpdateMaterialFlags()
{
	m_Flags &= ~(MTL_FLAG_REQUIRE_FORWARD_RENDERING | MTL_FLAG_REQUIRE_NEAREST_CUBEMAP);

	if (m_shaderItem.m_pShader)
	{
		IRenderShaderResources* pRendShaderResources = m_shaderItem.m_pShaderResources;

		const bool bAlphaBlended = (m_shaderItem.m_pShader->GetFlags() & (EF_NODRAW | EF_DECAL)) || (pRendShaderResources && pRendShaderResources->IsTransparent());
		const bool bIsHair = (m_shaderItem.m_pShader->GetFlags2() & EF2_HAIR) != 0;
		const bool bIsGlass = m_shaderItem.m_pShader->GetShaderType() == eST_Glass;

		if (bAlphaBlended && !(m_shaderItem.m_pShader->GetFlags2() & EF2_NODRAW) && !(m_shaderItem.m_pShader->GetFlags() & EF_DECAL))
		{
			m_Flags |= MTL_FLAG_REQUIRE_FORWARD_RENDERING;
		}
		else if (bIsHair || bIsGlass)
		{
			m_Flags |= MTL_FLAG_REQUIRE_FORWARD_RENDERING;
		}

		if ((bAlphaBlended || bIsHair || bIsGlass) &&
		    (pRendShaderResources) &&
		    (pRendShaderResources->GetTexture(EFTT_ENV)) &&
		    (pRendShaderResources->GetTexture(EFTT_ENV)->m_Sampler.m_eTexType == eTT_NearestCube))
		{
			m_Flags |= MTL_FLAG_REQUIRE_NEAREST_CUBEMAP;
		}

		// Make sure to refresh sectors
		static i32 nLastUpdateFrameId = 0;
		if (gEnv->IsEditing() && GetTerrain() && GetVisAreaUpr() && nLastUpdateFrameId != GetRenderer()->GetFrameID())
		{
			GetTerrain()->MarkAllSectorsAsUncompiled();
			GetVisAreaUpr()->MarkAllSectorsAsUncompiled();
			nLastUpdateFrameId = GetRenderer()->GetFrameID();
		}

		if (m_shaderItem.m_pShader && !!(m_shaderItem.m_pShader->GetFlags() & (EF_REFRACTIVE | EF_FORCEREFRACTIONUPDATE)))
			SetFlags(GetFlags() | MTL_FLAG_REFRACTIVE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetShaderItem(const SShaderItem& _ShaderItem)
{
	if (_ShaderItem.m_pShader)
		_ShaderItem.m_pShader->AddRef();
	if (_ShaderItem.m_pShaderResources)
	{
		_ShaderItem.m_pShaderResources->AddRef();
		_ShaderItem.m_pShaderResources->SetMaterialName(m_sUniqueMaterialName);
	}

	SAFE_RELEASE(m_shaderItem.m_pShader);
	SAFE_RELEASE(m_shaderItem.m_pShaderResources);

	m_shaderItem = _ShaderItem;
	gEnv->pRenderer->UpdateShaderItem(&m_shaderItem, this);
	IncrementModificationId();

	UpdateMaterialFlags();
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::AssignShaderItem(const SShaderItem& _ShaderItem)
{
	//if (_ShaderItem.m_pShader)
	//	_ShaderItem.m_pShader->AddRef();
	if (_ShaderItem.m_pShaderResources)
	{
		_ShaderItem.m_pShaderResources->SetMaterialName(m_sUniqueMaterialName);
	}

	SAFE_RELEASE(m_shaderItem.m_pShader);
	SAFE_RELEASE(m_shaderItem.m_pShaderResources);

	m_shaderItem = _ShaderItem;
	gEnv->pRenderer->UpdateShaderItem(&m_shaderItem, this);
	IncrementModificationId();

	UpdateMaterialFlags();
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetSurfaceType(tukk sSurfaceTypeName)
{
	m_nSurfaceTypeId = 0;

	ISurfaceType* pSurfaceType = GetMatMan()->GetSurfaceTypeByName(sSurfaceTypeName, m_sMaterialName);
	if (pSurfaceType)
		m_nSurfaceTypeId = pSurfaceType->GetId();
}

ISurfaceType* CMatInfo::GetSurfaceType()
{
	return GetMatMan()->GetSurfaceType(m_nSurfaceTypeId, m_sMaterialName);
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetMatTemplate(tukk sMatTemplate)
{
#ifdef SUPPORT_MATERIAL_EDITING
	m_sMatTemplate = sMatTemplate;
#endif
}

IMaterial* CMatInfo::GetMatTemplate()
{
#ifdef SUPPORT_MATERIAL_EDITING
	if (!m_sMatTemplate.empty())
	{
		return GetMatMan()->LoadMaterial(m_sMatTemplate, false);
	}
#else
	DrxFatalError("CMatInfo::GetMatTemplate not supported on this platform");
#endif

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetSubMtlCount(i32 numSubMtl)
{
	AUTO_LOCK(GetSubMaterialResizeLock());
	m_Flags |= MTL_FLAG_MULTI_SUBMTL;
	m_subMtls.resize(numSubMtl);
}

//////////////////////////////////////////////////////////////////////////
bool CMatInfo::IsStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES], IRenderMesh* pRenderMesh) const
{
	if (pRenderMesh)
	{
		TRenderChunkArray* pChunks = &pRenderMesh->GetChunks();

		if (pChunks != NULL)
		{
			for (u32 i = 0, size = pChunks->size(); i < size; i++)
			{
				if (!AreChunkTexturesStreamedIn(&(*pChunks)[i], nMinPrecacheRoundIds))
					return false;
			}
		}

		pChunks = &pRenderMesh->GetChunksSkinned();
		for (u32 i = 0, size = pChunks->size(); i < size; i++)
		{
			if (!AreChunkTexturesStreamedIn(&(*pChunks)[i], nMinPrecacheRoundIds))
				return false;
		}
	}
	else
	{
		if (!AreChunkTexturesStreamedIn(NULL, nMinPrecacheRoundIds))
			return false;
	}

	return true;
}

bool CMatInfo::IsStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const
{
	if (m_Flags & MTL_FLAG_MULTI_SUBMTL)
	{
		for (i32 i = 0, num = m_subMtls.size(); i < num; i++)
		{
			if (!m_subMtls[i]->AreTexturesStreamedIn(nMinPrecacheRoundIds))
			{
				return false;
			}
		}

		return true;
	}

	return AreTexturesStreamedIn(nMinPrecacheRoundIds);
}

bool CMatInfo::AreChunkTexturesStreamedIn(CRenderChunk* pRenderChunk, i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const
{
	const CMatInfo* pMaterial = this;

	if (pRenderChunk && pRenderChunk->pRE && pRenderChunk->nNumIndices && pRenderChunk->nNumVerts)
	{
		// chunk is defined and have valid geometry
		if (pRenderChunk->m_nMatID < (u16)m_subMtls.size())
			pMaterial = (CMatInfo*)m_subMtls[pRenderChunk->m_nMatID];

		if (pMaterial != NULL)
			return pMaterial->AreTexturesStreamedIn(nMinPrecacheRoundIds);
	}
	else if (!pRenderChunk)
	{
		if (!AreTexturesStreamedIn(nMinPrecacheRoundIds))
			return false;

		for (i32 nSubMtl = 0; nSubMtl < (i32)m_subMtls.size(); ++nSubMtl)
		{
			pMaterial = m_subMtls[nSubMtl];

			if (pMaterial != NULL)
			{
				if (!pMaterial->AreTexturesStreamedIn(nMinPrecacheRoundIds))
					return false;
			}
		}
	}

	return true;
}

bool CMatInfo::AreTexturesStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const
{
	// Check this material
	if (IRenderShaderResources* pShaderResources = GetShaderItem().m_pShaderResources)
	{
		for (i32 t = 0; t < EFTT_MAX; t++)
		{
			if (SEfResTexture* pTextureResource = pShaderResources->GetTexture(t))
			{
				if (ITexture* pTexture = pTextureResource->m_Sampler.m_pITex)
				{
					if (!pTexture->IsStreamedIn(nMinPrecacheRoundIds))
						return false;
				}
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetSubMtl(i32 nSlot, IMaterial* pMtl)
{
	assert(nSlot >= 0 && nSlot < (i32)m_subMtls.size());
	m_subMtls[nSlot] = (CMatInfo*)pMtl;

	if (pMtl)
	{
		const auto& shaderItem = pMtl->GetShaderItem();
		DRX_ASSERT(shaderItem.m_pShader);
		if (shaderItem.m_pShader && !!(shaderItem.m_pShader->GetFlags() & (EF_REFRACTIVE | EF_FORCEREFRACTIONUPDATE)))
			SetFlags(GetFlags() | MTL_FLAG_REFRACTIVE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetLayerCount(u32 nCount)
{
	if (!m_pMaterialLayers)
	{
		m_pMaterialLayers = new MatLayers;
	}

	m_pMaterialLayers->resize(nCount);
}

//////////////////////////////////////////////////////////////////////////
u32 CMatInfo::GetLayerCount() const
{
	if (m_pMaterialLayers)
	{
		return (u32) m_pMaterialLayers->size();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetLayer(u32 nSlot, IMaterialLayer* pLayer)
{
	assert(m_pMaterialLayers);
	assert(nSlot < (u32) m_pMaterialLayers->size());

	if (m_pMaterialLayers && pLayer && nSlot < (u32) m_pMaterialLayers->size())
		(*m_pMaterialLayers)[nSlot] = static_cast<CMaterialLayer*>(pLayer);
}

//////////////////////////////////////////////////////////////////////////

const IMaterialLayer* CMatInfo::GetLayer(u8 nLayersMask, u8 nLayersUsageMask) const
{
	if (m_pMaterialLayers && nLayersMask)
	{
		i32 nSlotCount = m_pMaterialLayers->size();
		for (i32 nSlot = 0; nSlot < nSlotCount; ++nSlot)
		{
			CMaterialLayer* pLayer = static_cast<CMaterialLayer*>((*m_pMaterialLayers)[nSlot]);

			if (nLayersMask & (1 << nSlot))
			{
				if (pLayer)
				{
					m_pActiveLayer = pLayer;
					return m_pActiveLayer;
				}

				m_pActiveLayer = 0;

				return 0;
			}
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
const IMaterialLayer* CMatInfo::GetLayer(u32 nSlot) const
{
	if (m_pMaterialLayers && nSlot < (u32) m_pMaterialLayers->size())
		return static_cast<CMaterialLayer*>((*m_pMaterialLayers)[nSlot]);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
IMaterialLayer* CMatInfo::CreateLayer()
{
	return new CMaterialLayer;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetUserData(uk pUserData)
{
#ifdef SUPPORT_MATERIAL_EDITING
	m_pUserData = pUserData;
#endif
}

//////////////////////////////////////////////////////////////////////////
uk CMatInfo::GetUserData() const
{
#ifdef SUPPORT_MATERIAL_EDITING
	return m_pUserData;
#else
	DrxFatalError("CMatInfo::GetUserData not supported on this platform");
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
i32 CMatInfo::FillSurfaceTypeIds(i32 pSurfaceIdsTable[])
{
	if (m_subMtls.empty() || !(m_Flags & MTL_FLAG_MULTI_SUBMTL))
	{
		pSurfaceIdsTable[0] = m_nSurfaceTypeId;
		return 1; // Not Multi material.
	}
	for (i32 i = 0; i < (i32)m_subMtls.size(); i++)
	{
		if (m_subMtls[i] != 0)
			pSurfaceIdsTable[i] = m_subMtls[i]->m_nSurfaceTypeId;
		else
			pSurfaceIdsTable[i] = 0;
	}
	return m_subMtls.size();
}

void CMatInfo::Copy(IMaterial* pMtlDest, EMaterialCopyFlags flags)
{
	CMatInfo* pMatInfo = static_cast<CMatInfo*>(pMtlDest);
	if (flags & MTL_COPY_NAME)
	{
		pMatInfo->m_sMaterialName = m_sMaterialName;
		pMatInfo->m_sUniqueMaterialName = m_sUniqueMaterialName;
	}
	pMatInfo->m_nSurfaceTypeId = m_nSurfaceTypeId;
	pMatInfo->m_Flags = m_Flags;
#ifdef SUPPORT_MATERIAL_EDITING
	if (flags & MTL_COPY_TEMPLATE)
	{
		pMatInfo->m_sMatTemplate = m_sMatTemplate;
	}
#endif
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	pMatInfo->m_pConsoleMtl = m_pConsoleMtl;
#endif

	if (GetShaderItem().m_pShaderResources)
	{
		const SShaderItem& siSrc(GetShaderItem());
		SInputShaderResourcesPtr pIsr = GetRenderer()->EF_CreateInputShaderResource(siSrc.m_pShaderResources);
		
		const SShaderItem& siDstTex(pMatInfo->GetShaderItem());
		SInputShaderResourcesPtr idsTex = GetRenderer()->EF_CreateInputShaderResource(siDstTex.m_pShaderResources);

		if (!(flags & MTL_COPY_TEXTURES))
		{
			for (i32 n = 0; n < EFTT_MAX; n++)
			{
				pIsr->m_Textures[n] = idsTex->m_Textures[n];
			}
		}

		SShaderItem siDst(GetRenderer()->EF_LoadShaderItem(siSrc.m_pShader->GetName(), false, 0, pIsr, siSrc.m_pShader->GetGenerationMask()));
		siDst.m_pShaderResources->CloneConstants(siSrc.m_pShaderResources); // Lazy "Copy", stays a reference until changed, after which it becomes unlinked
		pMatInfo->AssignShaderItem(siDst);
	}
}

//////////////////////////////////////////////////////////////////////////
CMatInfo* CMatInfo::Clone(CMatInfo* pParentOfClonedMtl)
{
	CMatInfo* pMatInfo = new CMatInfo;

	pMatInfo->m_sMaterialName = m_sMaterialName;
	pMatInfo->m_sUniqueMaterialName = m_sUniqueMaterialName;
	pMatInfo->m_nSurfaceTypeId = m_nSurfaceTypeId;
	pMatInfo->m_Flags = m_Flags;
#ifdef SUPPORT_MATERIAL_EDITING
	pMatInfo->m_sMatTemplate = m_sMatTemplate;
#endif
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	pMatInfo->m_pConsoleMtl = m_pConsoleMtl;
#endif

	if (GetShaderItem().m_pShaderResources)
	{
		const SShaderItem& siSrc(GetShaderItem());

		SShaderItem siDst(siSrc.Clone());
		pMatInfo->AssignShaderItem(siDst);
	}

	return pMatInfo;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::GetMemoryUsage(IDrxSizer* pSizer) const
{
	SIZER_COMPONENT_NAME(pSizer, "Material");
	pSizer->AddObject(this, sizeof(*this));

	if (m_pMaterialLayers)
	{
		i32 n = m_pMaterialLayers->size();
		for (i32 i = 0; i < n; i++)
		{
			CMaterialLayer* pMaterialLayer = (*m_pMaterialLayers)[i];
			if (pMaterialLayer)
			{
				pMaterialLayer->GetMemoryUsage(pSizer);
			}
		}
	}

	// all sub material
	{
		pSizer->AddObject(m_subMtls);
	}
}
//////////////////////////////////////////////////////////////////////////
size_t CMatInfo::GetResourceMemoryUsage(IDrxSizer* pSizer)
{
	size_t nTotalResourceMemoryUsage(0);

	if (m_pMaterialLayers)
	{
		i32 n = m_pMaterialLayers->size();
		for (i32 i = 0; i < n; i++)
		{
			CMaterialLayer* pMaterialLayer = (*m_pMaterialLayers)[i];
			if (pMaterialLayer)
			{
				nTotalResourceMemoryUsage += pMaterialLayer->GetResourceMemoryUsage(pSizer);
			}
		}
	}

	if (m_shaderItem.m_pShaderResources)
	{
		nTotalResourceMemoryUsage += m_shaderItem.m_pShaderResources->GetResourceMemoryUsage(pSizer);
	}

	// all sub material
	{
		i32 iCnt = GetSubMtlCount();
		for (i32 i = 0; i < iCnt; ++i)
		{
			IMaterial* piMaterial(GetSubMtl(i));
			if (piMaterial)
			{
				nTotalResourceMemoryUsage += piMaterial->GetResourceMemoryUsage(pSizer);
			}
		}
	}
	return nTotalResourceMemoryUsage;
}

//////////////////////////////////////////////////////////////////////////
bool CMatInfo::SetGetMaterialParamFloat(tukk sParamName, float& v, bool bGet)
{
	if (!m_shaderItem.m_pShaderResources)
		return false;

	bool bEmissive = m_shaderItem.m_pShaderResources->IsEmissive();
	bool bOk = m_shaderItem.m_pShaderResources && GetMaterialHelpers().SetGetMaterialParamFloat(*m_shaderItem.m_pShaderResources, sParamName, v, bGet);
	if (bOk && m_shaderItem.m_pShader && !bGet)
	{
		// since "emissive_intensity" is a post effect it needs to be updated here
		if (bEmissive != m_shaderItem.m_pShaderResources->IsEmissive())
		{
			GetRenderer()->ForceUpdateShaderItem(&m_shaderItem, this);
			IncrementModificationId();
		}

		m_shaderItem.m_pShaderResources->UpdateConstants(m_shaderItem.m_pShader);
	}

	return bOk;
}

bool CMatInfo::SetGetMaterialParamVec3(tukk sParamName, Vec3& v, bool bGet)
{
	bool bOk = m_shaderItem.m_pShaderResources && GetMaterialHelpers().SetGetMaterialParamVec3(*m_shaderItem.m_pShaderResources, sParamName, v, bGet);
	if (bOk && m_shaderItem.m_pShader && !bGet)
	{
		m_shaderItem.m_pShaderResources->UpdateConstants(m_shaderItem.m_pShader);
	}

	return bOk;
}

void CMatInfo::SetTexture(i32 textureId, i32 textureSlot)
{
	ITexture* pTexture = gEnv->pRenderer->EF_GetTextureByID(textureId);
	if (pTexture)
	{
		ITexture*& pTex = m_shaderItem.m_pShaderResources->GetTexture(textureSlot)->m_Sampler.m_pITex;
		SAFE_RELEASE(pTex);
		pTex = pTexture;

		gEnv->pRenderer->ForceUpdateShaderItem(&m_shaderItem, this);
	}
}

void CMatInfo::SetSubTexture(i32 textureId, i32 subMaterialSlot, i32 textureSlot)
{
	ITexture* pTexture = gEnv->pRenderer->EF_GetTextureByID(textureId);
	if (pTexture)
	{
		if (IMaterial* pSubMtl = GetSubMtl(subMaterialSlot))
		{
			ITexture*& pTex = pSubMtl->GetShaderItem().m_pShaderResources->GetTexture(textureSlot)->m_Sampler.m_pITex;
			SAFE_RELEASE(pTex);
			pTex = pTexture;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetCamera(CCamera& cam)
{
	if (!m_shaderItem.m_pShaderResources)
	{
		return;
	}

	CCamera* pC = m_shaderItem.m_pShaderResources->GetCamera();
	if (!pC)
		pC = new CCamera;
	m_shaderItem.m_pShaderResources->SetCamera(pC);
}

//////////////////////////////////////////////////////////////////////////

tukk CMatInfo::GetLoadingCallstack()
{
#ifdef TRACE_MATERIAL_LEAKS
	return m_sLoadingCallstack.c_str();
#else
	return "";
#endif
}

void CMatInfo::PrecacheMaterial(const float _fEntDistance, IRenderMesh* pRenderMesh, bool bFullUpdate, bool bDrawNear)
{
	//	FUNCTION_PROFILER_3DENGINE;
	LOADING_TIME_PROFILE_SECTION;

	i32 nFlags = 0;
	float fEntDistance;
	if (bDrawNear)
	{
		fEntDistance = _fEntDistance;
		nFlags |= FPR_HIGHPRIORITY;
	}
	else
		fEntDistance = max(GetFloatCVar(e_StreamPredictionMinReportDistance), _fEntDistance);

	float fMipFactor = fEntDistance * fEntDistance;

	// updating texture streaming distances
	if (pRenderMesh)
	{
		TRenderChunkArray* pChunks = &pRenderMesh->GetChunks();

		if (pChunks != NULL)
		{
			for (u32 i = 0, size = pChunks->size(); i < size; i++)
			{
				PrecacheChunkTextures(fMipFactor, nFlags, &(*pChunks)[i], bFullUpdate);
			}
		}

		pChunks = &pRenderMesh->GetChunksSkinned();
		for (u32 i = 0, size = pChunks->size(); i < size; i++)
		{
			PrecacheChunkTextures(fMipFactor, nFlags, &(*pChunks)[i], bFullUpdate);
		}
	}
	else
	{
		PrecacheChunkTextures(fMipFactor, nFlags, NULL, bFullUpdate);
	}
}

void CMatInfo::RequestTexturesLoading(const float fMipFactor)
{
	PrecacheTextures(fMipFactor, FPR_STARTLOADING, false);
}

void CMatInfo::ForceTexturesLoading(const float fMipFactor)
{
	PrecacheTextures(fMipFactor, FPR_STARTLOADING + FPR_HIGHPRIORITY, true);
}

void CMatInfo::ForceTexturesLoading(i32k iScreenTexels)
{
	PrecacheTextures(iScreenTexels, FPR_STARTLOADING + FPR_HIGHPRIORITY, true);
}

void CMatInfo::PrecacheTextures(const float fMipFactor, i32k nFlags, bool bFullUpdate)
{
	SStreamingPredictionZone& rZone = m_streamZoneInfo[bFullUpdate ? 1 : 0];
	i32 bHighPriority = (nFlags & FPR_HIGHPRIORITY) != 0;

	rZone.fMinMipFactor = min(rZone.fMinMipFactor, fMipFactor);
	rZone.bHighPriority |= bHighPriority;

	i32 nRoundId = bFullUpdate ? GetObjUpr()->m_nUpdateStreamingPrioriryRoundIdFast : GetObjUpr()->m_nUpdateStreamingPrioriryRoundId;

	// TODO: fix fast update
	if (rZone.nRoundId != nRoundId)
	{
		i32 nCurrentFlags = Get3DEngine()->IsShadersSyncLoad() ? FPR_SYNCRONOUS : 0;
		nCurrentFlags |= bFullUpdate ? FPR_SINGLE_FRAME_PRIORITY_UPDATE : 0;

		SShaderItem& rSI = m_shaderItem;
		if (rSI.m_pShader && rSI.m_pShaderResources && !(rSI.m_pShader->GetFlags() & EF_NODRAW))
		{
			if (rZone.nRoundId == (nRoundId - 1))
			{
				nCurrentFlags |= rZone.bHighPriority ? FPR_HIGHPRIORITY : 0;
				GetRenderer()->EF_PrecacheResource(&rSI, rZone.fMinMipFactor, 0, nCurrentFlags, nRoundId, 1); // accumulated value is valid
			}
			else
			{
				nCurrentFlags |= (nFlags & FPR_HIGHPRIORITY);
				GetRenderer()->EF_PrecacheResource(&rSI, fMipFactor, 0, nCurrentFlags, nRoundId, 1); // accumulated value is not valid, pass current value
			}
		}

		rZone.nRoundId = nRoundId;
		rZone.fMinMipFactor = fMipFactor;
		rZone.bHighPriority = bHighPriority;
	}
}

void CMatInfo::PrecacheTextures(i32k iScreenTexels, i32k nFlags, bool bFullUpdate)
{
	i32 nRoundId = bFullUpdate ? GetObjUpr()->m_nUpdateStreamingPrioriryRoundIdFast : GetObjUpr()->m_nUpdateStreamingPrioriryRoundId;

	// TODO: fix fast update
	if (true)
	{
		i32 nCurrentFlags = Get3DEngine()->IsShadersSyncLoad() ? FPR_SYNCRONOUS : 0;
		nCurrentFlags |= bFullUpdate ? FPR_SINGLE_FRAME_PRIORITY_UPDATE : 0;

		SShaderItem& rSI = m_shaderItem;
		if (rSI.m_pShader && rSI.m_pShaderResources && !(rSI.m_pShader->GetFlags() & EF_NODRAW))
		{
			{
				nCurrentFlags |= (nFlags & FPR_HIGHPRIORITY);
				GetRenderer()->EF_PrecacheResource(&rSI, iScreenTexels, 0, nCurrentFlags, nRoundId, 1); // accumulated value is not valid, pass current value
			}
		}
	}
}

void CMatInfo::PrecacheChunkTextures(const float fMipFactorDef, i32k nFlags, CRenderChunk* pRenderChunk, bool bFullUpdate)
{
	//  FUNCTION_PROFILER_3DENGINE;

	CMatInfo* pMaterial = this;

	if (pRenderChunk && pRenderChunk->pRE && pRenderChunk->nNumIndices && pRenderChunk->nNumVerts)
	{
		// chunk is defined and have valid geometry
		if (pRenderChunk->m_nMatID < (u16)m_subMtls.size())
		{
			pMaterial = (CMatInfo*)m_subMtls[pRenderChunk->m_nMatID];
		}

		if (pMaterial != NULL)
		{
			float fMipFactor = GetCVars()->e_StreamPredictionTexelDensity ? (fMipFactorDef * pRenderChunk->m_texelAreaDensity) : fMipFactorDef;
			pMaterial->PrecacheTextures(fMipFactor, nFlags, bFullUpdate);
		}
	}
	else if (!pRenderChunk)
	{
		// chunk is not set - load all sub-materials
		float fMipFactor = fMipFactorDef;

		PrecacheTextures(fMipFactor, nFlags, bFullUpdate);

		for (i32 nSubMtl = 0; nSubMtl < (i32)m_subMtls.size(); ++nSubMtl)
		{
			pMaterial = m_subMtls[nSubMtl];

			if (pMaterial != NULL)
			{
				pMaterial->PrecacheTextures(fMipFactor, nFlags, bFullUpdate);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
i32 CMatInfo::GetTextureMemoryUsage(IDrxSizer* pSizer, i32 nSubMtlSlot)
{
	i32 textureSize = 0;
	std::set<ITexture*> used;

	i32 nSlotStart = 0;
	i32 nSlotEnd = (i32)m_subMtls.size();

	if (nSubMtlSlot >= 0)
	{
		nSlotStart = nSubMtlSlot;
		nSlotEnd = nSubMtlSlot + 1;
	}
	if (nSlotEnd >= (i32)m_subMtls.size())
		nSlotEnd = (i32)m_subMtls.size();

	if (nSlotEnd == 0)
		nSlotEnd = 1;

	for (i32 i = nSlotStart; i < nSlotEnd; i++)
	{
		IRenderShaderResources* pRes = m_shaderItem.m_pShaderResources;
		if (i < (i32)m_subMtls.size() && m_subMtls[i] != NULL && (m_Flags & MTL_FLAG_MULTI_SUBMTL))
		{
			SShaderItem shaderItem = m_subMtls[i]->m_shaderItem;
			if (!shaderItem.m_pShaderResources)
				continue;
			pRes = shaderItem.m_pShaderResources;
		}
		if (!pRes)
			continue;

		for (i32 j = 0; j < EFTT_MAX; j++)
		{
			SEfResTexture* pResTexure = pRes->GetTexture(j);
			if (!pResTexure || !pResTexure->m_Sampler.m_pITex)
				continue;

			ITexture* pTexture = pResTexure->m_Sampler.m_pITex;
			if (!pTexture)
				continue;

			if (used.find(pTexture) != used.end()) // Already used in size calculation.
				continue;
			used.insert(pTexture);

			i32 nTexSize = pTexture->GetDataSize();
			textureSize += nTexSize;

			if (pSizer)
				pSizer->AddObject(pTexture, nTexSize);
		}
	}

	return textureSize;
}

void CMatInfo::SetKeepLowResSysCopyForDiffTex()
{
	i32 nSlotStart = 0;
	i32 nSlotEnd = (i32)m_subMtls.size();

	if (nSlotEnd == 0)
		nSlotEnd = 1;

	for (i32 i = nSlotStart; i < nSlotEnd; i++)
	{
		IRenderShaderResources* pRes = m_shaderItem.m_pShaderResources;

		if (i < (i32)m_subMtls.size() && m_subMtls[i] != NULL && (m_Flags & MTL_FLAG_MULTI_SUBMTL))
		{
			SShaderItem shaderItem = m_subMtls[i]->m_shaderItem;
			if (!shaderItem.m_pShaderResources)
				continue;
			pRes = shaderItem.m_pShaderResources;
		}

		if (!pRes)
			continue;

#ifndef FEATURE_SVO_GI_ALLOW_HQ
		// For non HQ mode (consoles) keep the texture only if material is going to use alpha value from it
		if ((pRes->GetAlphaRef() == 0.f) && (pRes->GetStrengthValue(EFTT_OPACITY) == 1.f))
			continue;
#endif
		
		i32 j = EFTT_DIFFUSE;
		{
			SEfResTexture* pResTexure = pRes->GetTexture(j);
			if (!pResTexure || !pResTexure->m_Sampler.m_pITex)
				continue;
			ITexture* pTexture = pResTexure->m_Sampler.m_pITex;
			if (!pTexture)
				continue;

			pTexture->SetKeepSystemCopy(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::SetMaterialLinkName(tukk name)
{
#ifdef SUPPORT_MATERIAL_EDITING
	if (name)
	{
		m_sMaterialLinkName = name;
	}
	else
	{
		m_sMaterialLinkName.clear();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
tukk CMatInfo::GetMaterialLinkName() const
{
#ifdef SUPPORT_MATERIAL_EDITING
	return m_sMaterialLinkName.c_str();
#else
	DrxFatalError("CMatInfo::GetMaterialLinkName not supported on this platform");
	return "";
#endif
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::ActivateDynamicTextureSources(bool activate)
{
	i32k numSubMtls = max(GetSubMtlCount(), 1);
	for (i32 subMtlId = 0; subMtlId < numSubMtls; ++subMtlId)
	{
		IMaterial* pSubMtl = GetSafeSubMtl(subMtlId);
		if (pSubMtl)
		{
			const SShaderItem& shaderItem = pSubMtl->GetShaderItem();
			if (shaderItem.m_pShaderResources)
			{
				for (i32 texSlot = 0; texSlot < EFTT_MAX; ++texSlot)
				{
					SEfResTexture* pTex = shaderItem.m_pShaderResources->GetTexture(texSlot);
					if (pTex)
					{
						IDynTextureSource* pDynTexSrc = pTex->m_Sampler.m_pDynTexSource;
						if (pDynTexSrc)
						{
							pDynTexSrc->Activate(activate);
						}
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
DrxCriticalSection& CMatInfo::GetSubMaterialResizeLock()
{
	static DrxCriticalSection s_sSubMaterialResizeLock;
	return s_sSubMaterialResizeLock;
}

//////////////////////////////////////////////////////////////////////////
void CMatInfo::UpdateShaderItems()
{
	IRenderer* pRenderer = gEnv->pRenderer;
	pRenderer->UpdateShaderItem(&m_shaderItem, this);

	for (i32 i = 0; i < (i32)m_subMtls.size(); ++i)
	{
		CMatInfo* pSubMaterial = m_subMtls[i];
		if (pSubMaterial)
			pRenderer->UpdateShaderItem(&pSubMaterial->m_shaderItem, this);
	}
}

void CMatInfo::RefreshShaderResourceConstants()
{
	IRenderer* pRenderer = gEnv->pRenderer;
	if (!pRenderer)
		return;

	pRenderer->RefreshShaderResourceConstants(&m_shaderItem, this);

	for (i32 i = 0; i < (i32)m_subMtls.size(); ++i)
	{
		CMatInfo* pSubMaterial = m_subMtls[i];
		if (pSubMaterial)
			pRenderer->RefreshShaderResourceConstants(&pSubMaterial->m_shaderItem, this);
	}
}

#if DRX_PLATFORM_WINDOWS
void CMatInfo::LoadConsoleMaterial()
{
	#if defined(ENABLE_CONSOLE_MTL_VIZ)
	string conMtlName = m_sMaterialName + "_con.mtl";
	if (gEnv->pDrxPak->FGetSize(conMtlName.c_str(), true))
	{
		conMtlName.resize(conMtlName.size() - 4);
		m_pConsoleMtl = Get3DEngine()->GetMatMan()->LoadMaterial(conMtlName.c_str(), false, false, IMaterialUpr::ELoadingFlagsPreviewMode);
	}
	#endif
}
#endif

#if defined(ENABLE_CONSOLE_MTL_VIZ)
i32 CMatInfo::ms_useConsoleMat = -1;

void CMatInfo::RegisterConsoleMatCVar()
{
	i32k defVal = gEnv->IsEditor() ? 0 : -1;
	REGISTER_CVAR2("e_UseConsoleMtl", &ms_useConsoleMat, defVal, VF_NULL,
	               "Console material visualization mode. Enable before level loading to be in effect.\n"
	               "-1 = fully disabled (default)\n"
	               " 0 = enabled but renders regular materials only\n"
	               " 1 = enabled and renders console materials where available");
}
#endif

///////////////////////////////////////////////////////////////////////////////
IMaterial* CMatMan::GetDefaultMaterial()
{
	return m_pDefaultMtl;
}

///////////////////////////////////////////////////////////////////////////////
IMaterial* CMatInfo::GetSafeSubMtl(i32 nSubMtlSlot)
{
	if (m_subMtls.empty() || !(m_Flags & MTL_FLAG_MULTI_SUBMTL))
	{
		return this; // Not Multi material.
	}
	if (nSubMtlSlot >= 0 && nSubMtlSlot < (i32)m_subMtls.size() && m_subMtls[nSubMtlSlot] != NULL)
		return m_subMtls[nSubMtlSlot];
	else
		return GetMatMan()->GetDefaultMaterial();
}

///////////////////////////////////////////////////////////////////////////////
SShaderItem& CMatInfo::GetShaderItem()
{
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	if (ms_useConsoleMat == 1 && m_pConsoleMtl)
	{
		IMaterial* pConsoleMaterial = m_pConsoleMtl.get();
		return static_cast<CMatInfo*>(pConsoleMaterial)->CMatInfo::GetShaderItem();
	}
#endif

	return m_shaderItem;
}

///////////////////////////////////////////////////////////////////////////////
const SShaderItem& CMatInfo::GetShaderItem() const
{
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	if (ms_useConsoleMat == 1 && m_pConsoleMtl)
	{
		IMaterial* pConsoleMaterial = m_pConsoleMtl.get();
		return static_cast<CMatInfo*>(pConsoleMaterial)->CMatInfo::GetShaderItem();
	}
#endif

	return m_shaderItem;
}

//////////////////////////////////////////////////////////////////////////
SShaderItem& CMatInfo::GetShaderItem(i32 nSubMtlSlot)
{
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	if (ms_useConsoleMat == 1 && m_pConsoleMtl)
	{
		IMaterial* pConsoleMaterial = m_pConsoleMtl.get();
		return static_cast<CMatInfo*>(pConsoleMaterial)->CMatInfo::GetShaderItem(nSubMtlSlot);
	}
#endif

	SShaderItem* pShaderItem = NULL;
	if (m_subMtls.empty() || !(m_Flags & MTL_FLAG_MULTI_SUBMTL))
	{
		pShaderItem = &m_shaderItem; // Not Multi material.
	}
	else if (nSubMtlSlot >= 0 && nSubMtlSlot < (i32)m_subMtls.size() && m_subMtls[nSubMtlSlot] != NULL)
	{
		pShaderItem = &(m_subMtls[nSubMtlSlot]->m_shaderItem);
	}
	else
	{
		IMaterial* pDefaultMaterial = GetMatMan()->GetDefaultMaterial();
		pShaderItem = &(static_cast<CMatInfo*>(pDefaultMaterial)->m_shaderItem);
	}

	return *pShaderItem;
}

///////////////////////////////////////////////////////////////////////////////
const SShaderItem& CMatInfo::GetShaderItem(i32 nSubMtlSlot) const
{
#if defined(ENABLE_CONSOLE_MTL_VIZ)
	if (ms_useConsoleMat == 1 && m_pConsoleMtl)
	{
		IMaterial* pConsoleMaterial = m_pConsoleMtl.get();
		return static_cast<CMatInfo*>(pConsoleMaterial)->CMatInfo::GetShaderItem(nSubMtlSlot);
	}
#endif

	const SShaderItem* pShaderItem = NULL;
	if (m_subMtls.empty() || !(m_Flags & MTL_FLAG_MULTI_SUBMTL))
	{
		pShaderItem = &m_shaderItem; // Not Multi material.
	}
	else if (nSubMtlSlot >= 0 && nSubMtlSlot < (i32)m_subMtls.size() && m_subMtls[nSubMtlSlot] != NULL)
	{
		pShaderItem = &(m_subMtls[nSubMtlSlot]->m_shaderItem);
	}
	else
	{
		IMaterial* pDefaultMaterial = GetMatMan()->GetDefaultMaterial();
		pShaderItem = &(static_cast<CMatInfo*>(pDefaultMaterial)->m_shaderItem);
	}

	return *pShaderItem;
}

///////////////////////////////////////////////////////////////////////////////
bool CMatInfo::IsForwardRenderingRequired()
{
	bool bRequireForwardRendering = (m_Flags & MTL_FLAG_REQUIRE_FORWARD_RENDERING) != 0;

	if (!bRequireForwardRendering)
	{
		for (i32 i = 0; i < (i32)m_subMtls.size(); ++i)
		{
			if (m_subMtls[i] != 0 && m_subMtls[i]->m_Flags & MTL_FLAG_REQUIRE_FORWARD_RENDERING)
			{
				bRequireForwardRendering = true;
				break;
			}
		}
	}

	return bRequireForwardRendering;
}

///////////////////////////////////////////////////////////////////////////////
bool CMatInfo::IsNearestCubemapRequired()
{
	bool bRequireNearestCubemap = (m_Flags & MTL_FLAG_REQUIRE_NEAREST_CUBEMAP) != 0;

	if (!bRequireNearestCubemap)
	{
		for (i32 i = 0; i < (i32)m_subMtls.size(); ++i)
		{
			if (m_subMtls[i] != 0 && m_subMtls[i]->m_Flags & MTL_FLAG_REQUIRE_NEAREST_CUBEMAP)
			{
				bRequireNearestCubemap = true;
				break;
			}
		}
	}

	return bRequireNearestCubemap;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
