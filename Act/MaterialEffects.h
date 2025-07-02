// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Act/IMaterialEffects.h>
#include <drx3D/CoreX/Containers/DrxListenerSet.h>

#include "MFXLibrary.h"
#include "MFXContainer.h"
#include "SurfacesLookupTable.h"

class CMaterialFGUpr;
class CScriptBind_MaterialEffects;

namespace MaterialEffectsUtils
{
class CVisualDebug;
}

class CMaterialEffects : public IMaterialEffects
{
private:
	friend class CScriptBind_MaterialEffects;

	static i32k kMaxSurfaceCount = 255 + 1; // from SurfaceTypeUpr.h, but will also work with any other number
	typedef IEntityClass*                                                TPtrIndex;
	typedef i32                                                          TIndex;
	typedef std::pair<TIndex, TIndex>                                    TIndexPair;

	typedef std::vector<TIndex>                                          TSurfaceIdToLookupTable;
	typedef std::map<TMFXNameId, CMFXLibrary, stl::less_stricmp<string>> TFXLibrariesMap;
	typedef std::map<const TPtrIndex, TIndex>                            TPointerToLookupTable;
	typedef std::map<string, TIndex, stl::less_stricmp<string>>          TCustomNameToLookupTable;
	typedef std::vector<TMFXContainerPtr>                                TMFXContainers;
	typedef CListenerSet<IMaterialEffectsListener*>                      TEffectsListeners;

	struct SDelayedEffect
	{
		SDelayedEffect()
			: m_delay(0.0f)
		{

		}

		SDelayedEffect(TMFXContainerPtr pEffectContainer, const SMFXRunTimeEffectParams& runtimeParams)
			: m_pEffectContainer(pEffectContainer)
			, m_effectRuntimeParams(runtimeParams)
			, m_delay(pEffectContainer->GetParams().delay)
		{
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(m_pEffectContainer);
		}

		TMFXContainerPtr        m_pEffectContainer;
		SMFXRunTimeEffectParams m_effectRuntimeParams;
		float                   m_delay;
	};

	typedef std::vector<SDelayedEffect> TDelayedEffects;

public:
	CMaterialEffects();
	virtual ~CMaterialEffects();

	// load flowgraph based effects
	bool LoadFlowGraphLibs();

	// serialize
	void Serialize(TSerialize ser);

	// load assets referenced by material effects.
	void PreLoadAssets();

	// IMaterialEffects
	virtual void                LoadFXLibraries();
	virtual void                Reset(bool bCleanup);
	virtual void                ClearDelayedEffects();
	virtual TMFXEffectId        GetEffectIdByName(tukk libName, tukk effectName);
	virtual TMFXEffectId        GetEffectId(i32 surfaceIndex1, i32 surfaceIndex2);
	virtual TMFXEffectId        GetEffectId(tukk customName, i32 surfaceIndex2);
	virtual TMFXEffectId        GetEffectId(IEntityClass* pEntityClass, i32 surfaceIndex2);
	virtual SMFXResourceListPtr GetResources(TMFXEffectId effectId) const;
	virtual bool                ExecuteEffect(TMFXEffectId effectId, SMFXRunTimeEffectParams& runtimeParams);
	virtual void                StopEffect(TMFXEffectId effectId);
	virtual i32                 GetDefaultSurfaceIndex() { return m_defaultSurfaceId; }
	virtual i32                 GetDefaultCanopyIndex();
	virtual bool                PlayBreakageEffect(ISurfaceType* pSurfaceType, tukk breakageType, const SMFXBreakageParams& breakageParams);
	virtual void                SetCustomParameter(TMFXEffectId effectId, tukk customParameter, const SMFXCustomParamValue& customParameterValue);
	virtual void                CompleteInit();
	virtual void                ReloadMatFXFlowGraphs();
	virtual size_t              GetMatFXFlowGraphCount() const;
	virtual IFlowGraphPtr       GetMatFXFlowGraph(i32 index, string* pFileName = NULL) const;
	virtual IFlowGraphPtr       LoadNewMatFXFlowGraph(const string& filename);
	virtual void                EnumerateEffectNames(EnumerateMaterialEffectsDataCallback& callback, tukk szLibraryName) const;
	virtual void                EnumerateLibraryNames(EnumerateMaterialEffectsDataCallback& callback) const;
	virtual void                LoadFXLibraryFromXMLInMemory(tukk szName, XmlNodeRef root);
	virtual void                UnloadFXLibrariesWithPrefix(tukk szName);
	virtual void                SetAnimFXEvents(IAnimFXEvents* pAnimEvents)                        { m_pAnimFXEvents = pAnimEvents; }
	virtual IAnimFXEvents*      GetAnimFXEvents()                                                  { return m_pAnimFXEvents; }
	virtual void                AddListener(IMaterialEffectsListener* pListener, tukk name) { m_listeners.Add(pListener, name); }
	virtual void                RemoveListener(IMaterialEffectsListener* pListener)                { m_listeners.Remove(pListener); }
	// ~IMaterialEffects

	void                GetMemoryUsage(IDrxSizer* s) const;
	void                NotifyFGHudEffectEnd(IFlowGraphPtr pFG);
	void                Update(float frameTime);
	void                SetUpdateMode(bool bMode);
	CMaterialFGUpr* GetFGUpr() const { return m_pMaterialFGUpr; }

	void                FullReload();

private:

	// Loading data from files
	void LoadSpreadSheet();
	void LoadFXLibrary(tukk name);

	// schedule effect
	void TimedEffect(TMFXContainerPtr pEffectContainer, const SMFXRunTimeEffectParams& params);

	// index1 x index2 are used to lookup in m_matmat array
	inline TMFXEffectId InternalGetEffectId(i32 index1, i32 index2) const
	{
		const TMFXEffectId effectId = m_surfacesLookupTable.GetValue(index1, index2, InvalidEffectId);
		return effectId;
	}

	inline TMFXContainerPtr InternalGetEffect(tukk libName, tukk effectName) const
	{
		TFXLibrariesMap::const_iterator iter = m_mfxLibraries.find(CONST_TEMP_STRING(libName));
		if (iter != m_mfxLibraries.end())
		{
			const CMFXLibrary& mfxLibrary = iter->second;
			return mfxLibrary.FindEffectContainer(effectName);
		}
		return 0;
	}

	inline TMFXContainerPtr InternalGetEffect(TMFXEffectId effectId) const
	{
		assert(effectId < m_effectContainers.size());
		if (effectId < m_effectContainers.size())
			return m_effectContainers[effectId];
		return 0;
	}

	inline TIndex SurfaceIdToMatrixEntry(i32 surfaceIndex)
	{
		return (surfaceIndex >= 0 && surfaceIndex < m_surfaceIdToMatrixEntry.size()) ? m_surfaceIdToMatrixEntry[surfaceIndex] : 0;
	}

private:

	i32                 m_defaultSurfaceId;
	i32                 m_canopySurfaceId;
	bool                m_bUpdateMode;
	bool                m_bDataInitialized;
	CMaterialFGUpr* m_pMaterialFGUpr;
	IAnimFXEvents*      m_pAnimFXEvents;

	// other systems can respond to internal events
	TEffectsListeners m_listeners;

	// The libraries we have loaded
	TFXLibrariesMap m_mfxLibraries;

	// This maps a surface type to the corresponding column(==row) in the matrix
	TSurfaceIdToLookupTable m_surfaceIdToMatrixEntry;

	// This maps custom pointers (entity classes)
	TPointerToLookupTable m_ptrToMatrixEntryMap;

	// Converts custom tags to indices
	TCustomNameToLookupTable m_customConvert;

	// Main lookup surface x surface -> effectId
	MaterialEffectsUtils::SSurfacesLookupTable<TMFXEffectId> m_surfacesLookupTable;

	// All effect containers (indexed by TEffectId)
	TMFXContainers m_effectContainers;

	// runtime effects which are delayed
	TDelayedEffects m_delayedEffects;

#ifdef MATERIAL_EFFECTS_DEBUG
	friend class MaterialEffectsUtils::CVisualDebug;

	MaterialEffectsUtils::CVisualDebug* m_pVisualDebug;
#endif
};
