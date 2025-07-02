// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   MatMan.h
//  Version:     v1.00
//  Created:     23/8/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __MatMan_h__
#define __MatMan_h__
#pragma once

#include <drx3D/Eng3D/DinrusX3dEngBase.h>
#include <drx3D/Eng3D/SurfaceTypeUpr.h>
#include <drx3D/CoreX/Thread/DrxThreadSafeRendererContainer.h>
#include <drx3D/Eng3D/MaterialHelpers.h>

#define MATERIAL_DELETION_DELAY 3 // Number of frames delay + 1

//  предварительные объявления.
struct IMaterial;
struct ISurfaceType;
struct ISurfaceTypeUpr;
class CMatInfo;

//////////////////////////////////////////////////////////////////////////
//
// CMatMan is a material manager class.
//
//////////////////////////////////////////////////////////////////////////
class CMatMan : public IMaterialUpr, public DinrusX3dEngBase
{
public:
	CMatMan();
	virtual ~CMatMan();

	void ShutDown();

	// interface IMaterialUpr --------------------------------------------------------

	virtual IMaterial* CreateMaterial(tukk sMtlName, i32 nMtlFlags = 0);
	virtual IMaterial* FindMaterial(tukk sMtlName) const;
	virtual IMaterial* LoadMaterial(tukk sMtlName, bool bMakeIfNotFound = true, bool bNonremovable = false, u64 nLoadingFlags = 0);
	virtual IMaterial* LoadMaterialFromXml(tukk sMtlName, XmlNodeRef mtlNode);
	virtual void       SetListener(IMaterialUprListener* pListener) { m_pListener = pListener; };
	virtual IMaterial* GetDefaultMaterial();
	virtual IMaterial* GetDefaultTerrainLayerMaterial()
	{
		if (!m_bInitialized)
		{
			InitDefaults();
		}
		return m_pDefaultTerrainLayersMtl;
	}
	virtual IMaterial*    GetDefaultLayersMaterial();
	virtual IMaterial*    GetDefaultHelperMaterial();
	virtual ISurfaceType* GetSurfaceTypeByName(tukk sSurfaceTypeName, tukk sWhy = NULL);
	virtual i32           GetSurfaceTypeIdByName(tukk sSurfaceTypeName, tukk sWhy = NULL);
	virtual ISurfaceType* GetSurfaceType(i32 nSurfaceTypeId, tukk sWhy = NULL)
	{
		return m_pSurfaceTypeUpr->GetSurfaceTypeFast(nSurfaceTypeId, sWhy);
	}
	virtual ISurfaceTypeUpr* GetSurfaceTypeUpr() { return m_pSurfaceTypeUpr; }
	virtual IMaterial*           LoadCGFMaterial(tukk szMaterialName, tukk szCgfFilename, u64 nLoadingFlags = 0);
	virtual IMaterial*           CloneMaterial(IMaterial* pMtl, i32 nSubMtl = -1);
	virtual IMaterial*           CloneMultiMaterial(IMaterial* pMtl, tukk sSubMtlName = 0);
	virtual void                 GetLoadedMaterials(IMaterial** pData, u32& nObjCount) const;
	virtual void                 SetAltMaterialSuffix(tukk pSuffix);

	virtual void                 CopyMaterial(IMaterial* pMtlSrc, IMaterial* pMtlDest, EMaterialCopyFlags flags);

	virtual void                 RenameMaterial(IMaterial* pMtl, tukk sNewName);
	virtual void                 RefreshMaterialRuntime();
	// ------------------------------------------------------------------------------------

	void InitDefaults();

	void PreloadLevelMaterials();
	void DoLoadSurfaceTypesInInit(bool doLoadSurfaceTypesInInit);

	void UpdateShaderItems();
	void RefreshShaderResourceConstants();

	// Load all known game decal materials.
	void PreloadDecalMaterials();

	//////////////////////////////////////////////////////////////////////////
	ISurfaceType* GetSurfaceTypeFast(i32 nSurfaceTypeId, tukk sWhy = NULL) { return m_pSurfaceTypeUpr->GetSurfaceTypeFast(nSurfaceTypeId, sWhy); }

	virtual void  GetMemoryUsage(IDrxSizer* pSizer) const;

	void          DelayedMaterialDeletion();

private: // -----------------------------------------------------------------------------
	friend class CMatInfo;
	void        Unregister(CMatInfo* pMat);

	void        DelayedDelete(CMatInfo* pMat);
	void        ForceDelayedMaterialDeletion();

	static bool LoadMaterialShader(IMaterial* pMtl, IMaterial* pParentMtl, tukk sShader, uint64 nShaderGenMask, SInputShaderResources& sr, XmlNodeRef& publicsNode, u64 nLoadingFlags = 0);
	static bool LoadMaterialLayerSlot(u32 nSlot, IMaterial* pMtl, tukk szShaderName, SInputShaderResources& pBaseResources, XmlNodeRef& pPublicsNode, u8 nLayerFlags);
	static IMaterial* MakeMaterialFromXml(tukk sMtlName, tukk sMtlFilename, XmlNodeRef node, bool bForcePureChild, u16 sortPrio = 0, IMaterial* pExistingMtl = 0, u64 nLoadingFlags = 0, IMaterial* pParentMtl = 0);

	static void ParsePublicParams(SInputShaderResources& sr, XmlNodeRef paramsNode);
	static tukk UnifyName(tukk sMtlName);

	// Can be called after material creation and initialization, to inform editor that new material in engine exist.
	// Only used internally.
	void       NotifyCreateMaterial(IMaterial* pMtl);
	// Make a valid material from the XML node.

	void       FreeAllMaterials();

private:
	typedef std::map<string, IMaterial*> MtlNameMap;

	DrxCriticalSection                m_AccessLock;

	MtlNameMap                        m_mtlNameMap;                   //

	IMaterialUprListener*         m_pListener;                    //
	_smart_ptr<CMatInfo>              m_pDefaultMtl;                  //
	_smart_ptr<IMaterial>             m_pDefaultLayersMtl;            //
	_smart_ptr<CMatInfo>              m_pDefaultTerrainLayersMtl;     //
	_smart_ptr<CMatInfo>              m_pNoDrawMtl;                   //
	_smart_ptr<CMatInfo>              m_pDefaultHelperMtl;

	std::vector<_smart_ptr<CMatInfo>> m_nonRemovables;                //

	std::vector<CMatInfo*>            m_DelayedDeletionMtls[MATERIAL_DELETION_DELAY];

	CSurfaceTypeUpr*              m_pSurfaceTypeUpr;          //

	string                            m_altSuffix;

	//////////////////////////////////////////////////////////////////////////
	// Cached XML parser.
	_smart_ptr<IXmlParser> m_pXmlParser;

	bool                   m_bInitialized;
	bool                   m_bLoadSurfaceTypesInInit;
	u8                  m_nDelayedDeleteID;

public:
	// Global namespace "instance", not a class "instance", no member-variables, only const functions;
	// Used to encapsulate the material-definition/io into DinrusX3dEng (and make it plugable that way).
	static MaterialHelpers s_materialHelpers;
};

#endif // __MatMan_h__
