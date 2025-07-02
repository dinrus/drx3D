// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   Material.h
//  Version:     v1.00
//  Created:     3/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Material_h__
#define __Material_h__
#pragma once

#include <drx3D/Eng3D/IMaterial.h>

#if DRX_PLATFORM_DESKTOP
	#define TRACE_MATERIAL_LEAKS
	#define SUPPORT_MATERIAL_EDITING
#endif

class CMaterialLayer : public IMaterialLayer
{
public:
	CMaterialLayer() : m_nRefCount(0), m_nFlags(0)
	{
	}

	virtual ~CMaterialLayer()
	{
		SAFE_RELEASE(m_pShaderItem.m_pShader);
		SAFE_RELEASE(m_pShaderItem.m_pShaderResources);
	}

	virtual void AddRef()
	{
		m_nRefCount++;
	};

	virtual void Release()
	{
		if (--m_nRefCount <= 0)
		{
			delete this;
		}
	}

	virtual void Enable(bool bEnable = true)
	{
		m_nFlags |= (bEnable == false) ? MTL_LAYER_USAGE_NODRAW : 0;
	}

	virtual bool IsEnabled() const
	{
		return (m_nFlags & MTL_LAYER_USAGE_NODRAW) ? false : true;
	}

	virtual void FadeOut(bool bFadeOut = true)
	{
		m_nFlags |= (bFadeOut == false) ? MTL_LAYER_USAGE_FADEOUT : 0;
	}

	virtual bool DoesFadeOut() const
	{
		return (m_nFlags & MTL_LAYER_USAGE_FADEOUT) ? true : false;
	}

	virtual void               SetShaderItem(const IMaterial* pParentMtl, const SShaderItem& pShaderItem);

	virtual const SShaderItem& GetShaderItem() const
	{
		return m_pShaderItem;
	}

	virtual SShaderItem& GetShaderItem()
	{
		return m_pShaderItem;
	}

	virtual void SetFlags(u8 nFlags)
	{
		m_nFlags = nFlags;
	}

	virtual u8 GetFlags() const
	{
		return m_nFlags;
	}

	void   GetMemoryUsage(IDrxSizer* pSizer);
	size_t GetResourceMemoryUsage(IDrxSizer* pSizer);

private:
	u8       m_nFlags;
	i32         m_nRefCount;
	SShaderItem m_pShaderItem;
};

//////////////////////////////////////////////////////////////////////
class CMatInfo : public IMaterial, public stl::intrusive_linked_list_node<CMatInfo>, public DinrusX3dEngBase
{
public:
	CMatInfo();
	~CMatInfo();

	void         ShutDown();
	virtual bool IsValid() const;

	virtual void AddRef();
	virtual void Release();

	virtual i32  GetNumRefs() { return m_nRefCount; };

	//////////////////////////////////////////////////////////////////////////
	// IMaterial implementation
	//////////////////////////////////////////////////////////////////////////

	virtual IMaterialHelpers& GetMaterialHelpers();
	virtual IMaterialUpr* GetMaterialUpr();

	virtual void              SetName(tukk pName);
	virtual tukk       GetName() const     { return m_sMaterialName; };

	virtual void              SetFlags(i32 flags) { m_Flags = flags; };
	virtual i32               GetFlags() const    { return m_Flags; };

	// Returns true if this is the default material.
	virtual bool          IsDefault() const;

	virtual i32           GetSurfaceTypeId() const { return m_nSurfaceTypeId; };

	virtual void          SetSurfaceType(tukk sSurfaceTypeName);
	virtual ISurfaceType* GetSurfaceType();

	virtual void          SetMatTemplate(tukk sMatTemplate);
	virtual IMaterial*    GetMatTemplate();
	// shader item

	virtual void               SetShaderItem(const SShaderItem& _ShaderItem);
	// [Alexey] EF_LoadShaderItem return value with RefCount = 1, so if you'll use SetShaderItem after EF_LoadShaderItem use Assign function
	virtual void               AssignShaderItem(const SShaderItem& _ShaderItem);

	virtual SShaderItem&       GetShaderItem();
	virtual const SShaderItem& GetShaderItem() const;

	virtual SShaderItem&       GetShaderItem(i32 nSubMtlSlot);
	virtual const SShaderItem& GetShaderItem(i32 nSubMtlSlot) const;

	virtual bool               IsStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES], IRenderMesh* pRenderMesh) const;
	virtual bool               IsStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const;
	bool                       AreChunkTexturesStreamedIn(CRenderChunk* pChunk, i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const;
	bool                       AreTexturesStreamedIn(i32k nMinPrecacheRoundIds[MAX_STREAM_PREDICTION_ZONES]) const;

	//////////////////////////////////////////////////////////////////////////
	virtual bool SetGetMaterialParamFloat(tukk sParamName, float& v, bool bGet);
	virtual bool SetGetMaterialParamVec3(tukk sParamName, Vec3& v, bool bGet);
	virtual void SetTexture(i32 textureId, i32 textureSlot);
	virtual void SetSubTexture(i32 textureId, i32 subMaterialSlot, i32 textureSlot);

	virtual void SetCamera(CCamera& cam);

	//////////////////////////////////////////////////////////////////////////
	// Sub materials.
	//////////////////////////////////////////////////////////////////////////
	virtual void       SetSubMtlCount(i32 numSubMtl);
	virtual i32        GetSubMtlCount() { return m_subMtls.size(); }
	virtual IMaterial* GetSubMtl(i32 nSlot)
	{
		if (m_subMtls.empty() || !(m_Flags & MTL_FLAG_MULTI_SUBMTL))
			return 0; // Not Multi material.
		if (nSlot >= 0 && nSlot < (i32)m_subMtls.size())
			return m_subMtls[nSlot];
		else
			return 0;
	}
	virtual void       SetSubMtl(i32 nSlot, IMaterial* pMtl);
	virtual void       SetUserData(uk pUserData);
	virtual uk      GetUserData() const;

	virtual IMaterial* GetSafeSubMtl(i32 nSlot);
	virtual CMatInfo*  Clone(CMatInfo* pParentOfClonedMtl);
	virtual void       Copy(IMaterial* pMtlDest, EMaterialCopyFlags flags);

	virtual void       ActivateDynamicTextureSources(bool activate);

	//////////////////////////////////////////////////////////////////////////
	// Layers
	//////////////////////////////////////////////////////////////////////////
	virtual void                  SetLayerCount(u32 nCount);
	virtual u32                GetLayerCount() const;
	virtual void                  SetLayer(u32 nSlot, IMaterialLayer* pLayer);
	virtual const IMaterialLayer* GetLayer(u8 nLayersMask, u8 nLayersUsageMask) const;
	virtual const IMaterialLayer* GetLayer(u32 nSlot) const;
	virtual IMaterialLayer*       CreateLayer();

	// Fill i32 table with surface ids of sub materials.
	// Return number of filled items.
	i32            FillSurfaceTypeIds(i32 pSurfaceIdsTable[]);

	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual size_t GetResourceMemoryUsage(IDrxSizer* pSizer);

	void           UpdateShaderItems();
	void           RefreshShaderResourceConstants();

	virtual void IncrementModificationId() final { m_nModificationId++; }
	u32 GetModificationId() const { return m_nModificationId; }

	//////////////////////////////////////////////////////////////////////////

	// Check for specific rendering conditions (forward rendering/nearest cubemap requirement)
	bool IsForwardRenderingRequired();
	bool IsNearestCubemapRequired();

	//////////////////////////////////////////////////////////////////////////
	// Debug routines
	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetLoadingCallstack();  // trace leaking materials by callstack

	virtual void        RequestTexturesLoading(const float fMipFactor);
	virtual void        ForceTexturesLoading(const float fMipFactor);
	virtual void        ForceTexturesLoading(i32k iScreenTexels);

	virtual void        PrecacheMaterial(const float fEntDistance, struct IRenderMesh* pRenderMesh, bool bFullUpdate, bool bDrawNear = false);
	void                PrecacheTextures(const float fMipFactor, i32k nFlags, bool bFullUpdate);
	void                PrecacheTextures(i32k iScreenTexels, i32k nFlags, bool bFullUpdate);
	void                PrecacheChunkTextures(const float fInstanceDistance, i32k nFlags, CRenderChunk* pRenderChunk, bool bFullUpdate);

	virtual i32         GetTextureMemoryUsage(IDrxSizer* pSizer, i32 nSubMtlSlot = -1);
	virtual void        SetKeepLowResSysCopyForDiffTex();

	virtual void        SetMaterialLinkName(tukk name);
	virtual tukk GetMaterialLinkName() const;

#if DRX_PLATFORM_WINDOWS
	virtual void LoadConsoleMaterial();
#endif

	virtual DrxCriticalSection& GetSubMaterialResizeLock();
public:
	//////////////////////////////////////////////////////////////////////////
	// for debug purposes
	//////////////////////////////////////////////////////////////////////////
#ifdef TRACE_MATERIAL_LEAKS
	string m_sLoadingCallstack;
#endif

private:

	void UpdateMaterialFlags();

private:
	friend class CMatMan;
	friend class CMaterialLayer;

	//////////////////////////////////////////////////////////////////////////
	string m_sMaterialName;
	string m_sUniqueMaterialName;

	// Id of surface type assigned to this material.
	i32 m_nSurfaceTypeId;

	//! Number of references to this material.
	 i32 m_nRefCount;
	//! Material flags.
	//! @see EMatInfoFlags
	i32         m_Flags;

	bool m_bDeleted;
	bool m_bDeletePending;

	SShaderItem m_shaderItem;

	// Used to detect the cases when dependent permanent render objects have to be updated
	u32 m_nModificationId;

	//! Array of Sub materials.
	typedef DynArray<_smart_ptr<CMatInfo>> SubMtls;
	SubMtls m_subMtls;

#ifdef SUPPORT_MATERIAL_EDITING
	// User data used by Editor.
	uk  m_pUserData;

	string m_sMaterialLinkName;
	// name of mat templalte material
	string m_sMatTemplate;
#endif

	//! Material layers
	typedef std::vector<_smart_ptr<CMaterialLayer>> MatLayers;
	MatLayers* m_pMaterialLayers;

	//! Used for material layers
	mutable CMaterialLayer* m_pActiveLayer;

	struct SStreamingPredictionZone
	{
		i32   nRoundId      : 31;
		i32   bHighPriority : 1;
		float fMinMipFactor;
	} m_streamZoneInfo[2];

#if defined(ENABLE_CONSOLE_MTL_VIZ)
public:
	static void RegisterConsoleMatCVar();
	static bool IsConsoleMatModeEnabled() { return ms_useConsoleMat >= 0; }

private:
	static i32            ms_useConsoleMat;
	_smart_ptr<IMaterial> m_pConsoleMtl;
#endif
};

#endif // __Material_h__
