// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <drx3D/Act/StdAfx.h>
#include <limits.h>
#include <drx3D/CoreX/String/Path.h>
#include <drx3D/Act/MaterialEffectsCVars.h>
#include <drx3D/Act/MaterialEffects.h>
#include <drx3D/Act/MFXRandomEffect.h>
#include <drx3D/Act/MFXParticleEffect.h>
#include <drx3D/Act/MFXDecalEffect.h>
#include <drx3D/Act/MFXRandomEffect.h>
#include <drx3D/Act/MaterialFGUpr.h>
#include <drx3D/Act/MaterialEffectsDebug.h>
#include <drx3D/CoreX/Memory/PoolAllocator.h>

#define MATERIAL_EFFECTS_SPREADSHEET_FILE     "libs/materialeffects/materialeffects.xml"
#define MATERIAL_EFFECTS_LIBRARIES_FOLDER     "libs/materialeffects/fxlibs"

#define MATERIAL_EFFECTS_SURFACE_TYPE_DEFAULT "mat_default"
#define MATERIAL_EFFECTS_SURFACE_TYPE_CANOPY  "mat_canopy"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define IMPL_POOL_RETURNING(type, rtype)                       \
  static stl::PoolAllocatorNoMT<sizeof(type)> m_pool_ ## type; \
  rtype type::Create()                                         \
  {                                                            \
    return new(m_pool_ ## type.Allocate())type();              \
  }                                                            \
  void type::Destroy()                                         \
  {                                                            \
    this->~type();                                             \
    m_pool_ ## type.Deallocate(this);                          \
  }                                                            \
  void type::FreePool()                                        \
  {                                                            \
    m_pool_ ## type.FreeMemory();                              \
  }
#define IMPL_POOL(type) IMPL_POOL_RETURNING(type, type*)

IMPL_POOL_RETURNING(SMFXResourceList, SMFXResourceListPtr);
IMPL_POOL(SMFXFlowGraphListNode);
IMPL_POOL(SMFXDecalListNode);
IMPL_POOL(SMFXParticleListNode);
IMPL_POOL(SMFXAudioListNode);
IMPL_POOL(SMFXForceFeedbackListNode);

namespace MaterialEffectsUtils
{
i32 FindSurfaceIdByName(tukk surfaceTypeName)
{
	DRX_ASSERT(surfaceTypeName != NULL);

	ISurfaceType* pSurfaceType = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceTypeUpr()->GetSurfaceTypeByName(surfaceTypeName);
	if (pSurfaceType != NULL)
	{
		return (i32)pSurfaceType->GetId();
	}

	return -1;
}
}

CMaterialEffects::CMaterialEffects()
	: m_bDataInitialized(false)
	, m_listeners(4)
	, m_pAnimFXEvents(nullptr)
{
	m_bUpdateMode = false;
	m_defaultSurfaceId = MaterialEffectsUtils::FindSurfaceIdByName(MATERIAL_EFFECTS_SURFACE_TYPE_DEFAULT);
	m_canopySurfaceId = m_defaultSurfaceId;
	m_pMaterialFGUpr = new CMaterialFGUpr();

#ifdef MATERIAL_EFFECTS_DEBUG
	m_pVisualDebug = new MaterialEffectsUtils::CVisualDebug();
#endif
}

CMaterialEffects::~CMaterialEffects()
{
	// smart pointers will automatically release.
	// the clears will immediately delete all effects while CMaterialEffects is still exisiting
	m_mfxLibraries.clear();
	m_delayedEffects.clear();
	m_effectContainers.clear();
	SAFE_DELETE(m_pMaterialFGUpr);

#ifdef MATERIAL_EFFECTS_DEBUG
	SAFE_DELETE(m_pVisualDebug);
#endif
}

void CMaterialEffects::LoadFXLibraries()
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "MaterialEffects");

	m_mfxLibraries.clear();
	m_effectContainers.clear();
	m_effectContainers.push_back(0); // 0 -> invalid effect id

	IDrxPak* pak = gEnv->pDrxPak;
	_finddata_t fd;

	stack_string searchPath;
	searchPath.Format("%s/*.xml", MATERIAL_EFFECTS_LIBRARIES_FOLDER);
	intptr_t handle = pak->FindFirst(searchPath.c_str(), &fd);
	i32 res = 0;
	if (handle != -1)
	{
		do
		{
			LoadFXLibrary(fd.name);
			res = pak->FindNext(handle, &fd);
			SLICE_AND_SLEEP();
		}
		while (res >= 0);
		pak->FindClose(handle);
	}

	m_canopySurfaceId = MaterialEffectsUtils::FindSurfaceIdByName(MATERIAL_EFFECTS_SURFACE_TYPE_CANOPY);

	// Notify external systems
	for (TEffectsListeners::Notifier notifier(m_listeners); notifier.IsValid(); notifier.Next())
	{
		notifier->OnPostLoadFXLibraries();
	}
}

void CMaterialEffects::LoadFXLibrary(tukk name)
{
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "FX Library XML (%s)", name);

	string path = PathUtil::Make(MATERIAL_EFFECTS_LIBRARIES_FOLDER, name);
	string fileName = name;

	i32 period = fileName.find(".");
	string libName = fileName.substr(0, period);

	XmlNodeRef libraryRootNode = gEnv->pSystem->LoadXmlFromFile(path);
	if (libraryRootNode != 0)
	{
		TFXLibrariesMap::iterator libIter = m_mfxLibraries.find(libName);
		if (libIter != m_mfxLibraries.end())
		{
			GameWarning("[MatFX]: Library '%s' already exists, skipping library file loading '%s'", libName.c_str(), path.c_str());
			return;
		}

		std::pair<TFXLibrariesMap::iterator, bool> iterPair = m_mfxLibraries.insert(TFXLibrariesMap::value_type(libName, CMFXLibrary()));
		assert(iterPair.second == true);
		libIter = iterPair.first;
		assert(libIter != m_mfxLibraries.end());

		const TMFXNameId& libraryNameId = libIter->first; // uses DrxString's ref-count feature
		CMFXLibrary& mfxLibrary = libIter->second;

		CMFXLibrary::SLoadingEnvironment libraryLoadingEnvironment(libraryNameId, libraryRootNode, m_effectContainers);
		mfxLibrary.LoadFromXml(libraryLoadingEnvironment);
	}
	else
	{
		GameWarning("[MatFX]: Failed to load library %s", path.c_str());
	}

}

bool CMaterialEffects::ExecuteEffect(TMFXEffectId effectId, SMFXRunTimeEffectParams& params)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return false;

	bool success = false;
	TMFXContainerPtr pEffectContainer = InternalGetEffect(effectId);
	if (pEffectContainer)
	{
		const float delay = pEffectContainer->GetParams().delay;
		if ((delay > 0.0f) && !(params.playflags & eMFXPF_Disable_Delay))
		{
			TimedEffect(pEffectContainer, params);
		}
		else
		{
			pEffectContainer->Execute(params);
		}
		success = true;

#ifdef MATERIAL_EFFECTS_DEBUG
		if (CMaterialEffectsCVars::Get().mfx_DebugVisual)
		{
			if (effectId != InvalidEffectId)
			{
				m_pVisualDebug->AddEffectDebugVisual(effectId, params);
			}
		}
#endif

	}

	return success;
}

void CMaterialEffects::StopEffect(TMFXEffectId effectId)
{
	TMFXContainerPtr pEffectContainer = InternalGetEffect(effectId);
	if (pEffectContainer)
	{
		SMFXResourceListPtr resources = SMFXResourceList::Create();
		pEffectContainer->GetResources(*resources);

		SMFXFlowGraphListNode* pNext = resources->m_flowGraphList;
		while (pNext)
		{
			GetFGUpr()->EndFGEffect(pNext->m_flowGraphParams.name);
			pNext = pNext->pNext;
		}
	}
}

void CMaterialEffects::SetCustomParameter(TMFXEffectId effectId, tukk customParameter, const SMFXCustomParamValue& customParameterValue)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return;

	TMFXContainerPtr pEffect = InternalGetEffect(effectId);
	if (pEffect)
	{
		pEffect->SetCustomParameter(customParameter, customParameterValue);
	}
}

namespace
{
struct CConstCharArray
{
	tukk ptr;
	size_t      count;
	CConstCharArray()
		: ptr("")
		, count(0)
	{
	}
};

struct less_CConstCharArray
{
	bool operator()(const CConstCharArray& s0, const CConstCharArray& s1) const
	{
		const size_t minCount = (s0.count < s1.count) ? s0.count : s1.count;
		i32k result = memicmp(s0.ptr, s1.ptr, minCount);
		return result ? (result < 0) : (s0.count < s1.count);
	}
};

void ToEffectString(const string& effectString, string& libName, string& effectName)
{
	size_t colon = effectString.find(':');
	if (colon != string::npos)
	{
		libName = effectString.substr(0, colon);
		effectName = effectString.substr(colon + 1, effectString.length() - colon - 1);
	}
	else
	{
		libName = effectString;
		effectName = effectString;
	}
}
}

void CMaterialEffects::LoadSpreadSheet()
{
	m_bDataInitialized = true;

	DrxComment("[MFX] Init");

	IXmlTableReader* const pXmlTableReader = gEnv->pSystem->GetXmlUtils()->CreateXmlTableReader();
	if (!pXmlTableReader)
	{
		GameWarning("[MFX] XML system failure");
		return;
	}

	// The root node.
	XmlNodeRef root = gEnv->pSystem->LoadXmlFromFile(MATERIAL_EFFECTS_SPREADSHEET_FILE);
	if (!root)
	{
		// The file wasn't found, or the wrong file format was used
		GameWarning("[MFX] File not found or wrong file type: %s", MATERIAL_EFFECTS_SPREADSHEET_FILE);
		pXmlTableReader->Release();
		return;
	}

	DrxComment("[MFX] Loaded: %s", MATERIAL_EFFECTS_SPREADSHEET_FILE);

	if (!pXmlTableReader->Begin(root))
	{
		GameWarning("[MFX] Table not found");
		pXmlTableReader->Release();
		return;
	}

	// temporary multimap: we store effectstring -> [TIndexPairs]+ there
	typedef std::vector<TIndexPair>                                        TIndexPairVec;
	typedef std::map<CConstCharArray, TIndexPairVec, less_CConstCharArray> TEffectStringToIndexPairVecMap;
	TEffectStringToIndexPairVecMap tmpEffectStringToIndexPairVecMap;

	i32 rowCount = 0;
	i32 warningCount = 0;

	CConstCharArray cell;
	string cellString; // temporary string

	// When we've gone down more rows than we have columns, we've entered special object space
	i32 maxCol = 0;
	std::vector<CConstCharArray> colOrder;
	std::vector<CConstCharArray> rowOrder;

	m_surfaceIdToMatrixEntry.resize(0);
	m_surfaceIdToMatrixEntry.resize(kMaxSurfaceCount, TIndex(0));
	m_ptrToMatrixEntryMap.clear();
	m_customConvert.clear();

	// Iterate through the table's rows
	for (;; )
	{
		i32 nRowIndex = -1;
		if (!pXmlTableReader->ReadRow(nRowIndex))
			break;

		// Iterate through the row's columns
		for (;; )
		{
			i32 colIndex = -1;
			if (!pXmlTableReader->ReadCell(colIndex, cell.ptr, cell.count))
				break;

			if (cell.count <= 0)
				continue;

			cellString.assign(cell.ptr, cell.count);

			if (rowCount == 0 && colIndex > 0)
			{
				i32k matId = gEnv->p3DEngine->GetMaterialUpr()->GetSurfaceTypeUpr()->GetSurfaceTypeByName(cellString.c_str(), "MFX", true)->GetId();
				if (matId != 0 || /* matId == 0 && */ stricmp(cellString.c_str(), "mat_default") == 0) // if matId != 0 or it's the mat_default name
				{
					// DrxLogAlways("[MFX] Material found: %s [ID=%d] [mapping to row/col=%d]", cellString.c_str(), matId, colCount);
					if (m_surfaceIdToMatrixEntry.size() < matId)
					{
						m_surfaceIdToMatrixEntry.resize(matId + 1);
						if (matId >= kMaxSurfaceCount)
						{
							assert(false && "MaterialEffects.cpp: SurfaceTypes exceeds 256. Reconsider implementation.");
							DrxLogAlways("MaterialEffects.cpp: SurfaceTypes exceeds %d. Reconsider implementation.", kMaxSurfaceCount);
						}
					}
					m_surfaceIdToMatrixEntry[matId] = colIndex;
				}
				else
				{
					GameWarning("MFX WARNING: Material not found: %s", cellString.c_str());
					++warningCount;
				}
				colOrder.push_back(cell);
			}
			else if (rowCount > 0 && colIndex > 0)
			{
				//DrxLog("[MFX] Event found: %s", cellString.c_str());
				tmpEffectStringToIndexPairVecMap[cell].push_back(TIndexPair(rowCount, colIndex));
			}
			else if (rowCount > maxCol && colIndex == 0)
			{
				IEntityClass* pEntityClass = gEnv->pEntitySystem->GetClassRegistry()->FindClass(cellString.c_str());
				//DrxLog("[MFX] Object class ID: %d", (i32)pEntityClass);
				if (pEntityClass)
				{
					// DrxComment("[MFX] Found EntityClass based entry: %s [mapping to row/col=%d]", cellString.c_str(), rowCount);
					m_ptrToMatrixEntryMap[pEntityClass] = rowCount;
				}
				else
				{
					// DrxComment("[MFX] Found Custom entry: %s [mapping to row/col=%d]", cellString.c_str(), rowCount);
					cellString.MakeLower();
					m_customConvert[cellString] = rowCount;
					++warningCount;
				}
			}
			else if (rowCount > 0 && colIndex == 0)
			{
				rowOrder.push_back(cell);
			}
			// Heavy-duty debug info
			//DrxLog("[MFX] celldata = %s at (%d, %d) rowCount=%d colIndex=%d maxCol=%d", curCellData->getContent(), i, j, rowCount, colIndex, maxCol);

			// Check if this is the furthest column we've seen thus far
			if (colIndex > maxCol)
				maxCol = colIndex;

			SLICE_AND_SLEEP();
		}
		// Increment row counter
		++rowCount;
	}

	// now postprocess the tmpEffectStringIndexPairVecMap and generate the m_matmatArray
	{
		// create the matmat array.
		// +1, because index pairs are in range [1..rowCount][1..maxCol]
		m_surfacesLookupTable.Create(rowCount + 1, maxCol + 1);
		TEffectStringToIndexPairVecMap::const_iterator iter = tmpEffectStringToIndexPairVecMap.begin();
		TEffectStringToIndexPairVecMap::const_iterator iterEnd = tmpEffectStringToIndexPairVecMap.end();
		string libName;
		string effectName;
		string tmpString;
		while (iter != iterEnd)
		{
			// lookup effect
			tmpString.assign(iter->first.ptr, iter->first.count);
			ToEffectString(tmpString, libName, effectName);
			TMFXContainerPtr pEffectContainer = InternalGetEffect(libName, effectName);
			TMFXEffectId effectId = pEffectContainer ? pEffectContainer->GetParams().effectId : InvalidEffectId;
			TIndexPairVec::const_iterator vecIter = iter->second.begin();
			TIndexPairVec::const_iterator vecIterEnd = iter->second.end();
			while (vecIter != vecIterEnd)
			{
				const TIndexPair& indexPair = *vecIter;
				// DrxLogAlways("[%d,%d]->%d '%s'", indexPair.first, indexPair.second, effectId, tmpString.c_str());
				m_surfacesLookupTable(indexPair.first, indexPair.second) = effectId;
				++vecIter;
			}
			++iter;
		}
	}

	if (CMaterialEffectsCVars::Get().mfx_Debug > 0)
	{
		DrxLogAlways("[MFX] RowCount=%d MaxCol=%d (*=%d)", rowCount, maxCol, rowCount * maxCol);
		for (i32 y = 0; y < m_surfacesLookupTable.m_nRows; ++y)
		{
			for (i32 x = 0; x < m_surfacesLookupTable.m_nCols; ++x)
			{
				TMFXEffectId idRowCol = m_surfacesLookupTable.GetValue(y, x, USHRT_MAX);
				assert(idRowCol != USHRT_MAX);
				TMFXContainerPtr pEffectRowCol = InternalGetEffect(idRowCol);
				if (pEffectRowCol)
				{
					DrxLogAlways("[%d,%d] -> %u '%s:%s'", y, x, idRowCol, pEffectRowCol->GetParams().libraryName.c_str(), pEffectRowCol->GetParams().name.c_str());
					if (y < m_surfacesLookupTable.m_nCols)
					{
						TMFXEffectId idColRow = m_surfacesLookupTable.GetValue(x, y, USHRT_MAX);
						assert(idColRow != USHRT_MAX);
						TMFXContainerPtr pEffectColRow = InternalGetEffect(idColRow);
						if (idRowCol != idColRow)
						{
							if (pEffectColRow)
							{
								GameWarning("[MFX] Identity mismatch: ExcelRowCol %d:%d: %s:%s != %s:%s", y + 1, x + 1,
								            pEffectRowCol->GetParams().libraryName.c_str(), pEffectRowCol->GetParams().name.c_str(),
								            pEffectColRow->GetParams().libraryName.c_str(), pEffectColRow->GetParams().name.c_str());
							}
							else
							{
								GameWarning("[MFX] Identity mismatch: ExcelRowCol %d:%d: %s:%s != [not found]", y + 1, x + 1,
								            pEffectRowCol->GetParams().libraryName.c_str(), pEffectRowCol->GetParams().name.c_str());
							}
						}
					}
				}
			}
		}
	}

	// check that we have the same number of columns and rows
	if (colOrder.size() > rowOrder.size())
	{
		GameWarning("[MFX] Found %d Columns, but not enough rows specified (%d)", (i32)colOrder.size(), (i32)rowOrder.size());
	}

	// check that column order matches row order
	if (CMaterialEffectsCVars::Get().mfx_Debug > 0)
	{
		string colName;
		string rowName;
		for (i32 i = 0; i < colOrder.size(); ++i)
		{
			colName.assign(colOrder[i].ptr, colOrder[i].count);
			if (i < rowOrder.size())
			{
				rowName.assign(rowOrder[i].ptr, rowOrder[i].count);
			}
			else
			{
				rowName.clear();
			}
			// DrxLogAlways("ExcelColRow=%d col=%s row=%s", i+2, colName.c_str(), rowName.c_str());
			if (colName != rowName)
			{
				GameWarning("ExcelColRow=%d: %s != %s", i + 2, colName.c_str(), rowName.c_str());
			}
		}
	}

	pXmlTableReader->Release();
}

void CMaterialEffects::PreLoadAssets()
{
	LOADING_TIME_PROFILE_SECTION;

	for (TMFXEffectId id = 0; id < m_effectContainers.size(); ++id)
		if (m_effectContainers[id])
			m_effectContainers[id]->PreLoadAssets();

	if (m_pMaterialFGUpr)
		return m_pMaterialFGUpr->PreLoad();
}

bool CMaterialEffects::LoadFlowGraphLibs()
{
	if (m_pMaterialFGUpr)
		return m_pMaterialFGUpr->LoadLibs();
	return false;
}

TMFXEffectId CMaterialEffects::GetEffectIdByName(tukk libName, tukk effectName)
{
	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return InvalidEffectId;

	const TMFXContainerPtr pEffectContainer = InternalGetEffect(libName, effectName);
	if (pEffectContainer)
	{
		return pEffectContainer->GetParams().effectId;
	}

	return InvalidEffectId;
}

TMFXEffectId CMaterialEffects::GetEffectId(i32 surfaceIndex1, i32 surfaceIndex2)
{
	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return InvalidEffectId;

	// Map surface IDs to internal matmat indices
	const TIndex idx1 = SurfaceIdToMatrixEntry(surfaceIndex1);
	const TIndex idx2 = SurfaceIdToMatrixEntry(surfaceIndex2);

#ifdef MATERIAL_EFFECTS_DEBUG
	TMFXEffectId effectId = InternalGetEffectId(idx1, idx2);

	if (CMaterialEffectsCVars::Get().mfx_DebugVisual)
	{
		if (effectId != InvalidEffectId)
		{
			m_pVisualDebug->AddLastSearchHint(effectId, surfaceIndex1, surfaceIndex2);
		}
		else
		{
			GameWarning("Could not find a valid effect at row %i and column %i of libs/materialeffects/materialeffects.xml", idx1, idx2);
		}
	}

	return effectId;
#else
	return InternalGetEffectId(idx1, idx2);
#endif

}

TMFXEffectId CMaterialEffects::GetEffectId(tukk customName, i32 surfaceIndex2)
{
	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return InvalidEffectId;

	const TIndex idx1 = stl::find_in_map(m_customConvert, CONST_TEMP_STRING(customName), 0);
	const TIndex idx2 = SurfaceIdToMatrixEntry(surfaceIndex2);

#ifdef MATERIAL_EFFECTS_DEBUG
	TMFXEffectId effectId = InternalGetEffectId(idx1, idx2);

	if (CMaterialEffectsCVars::Get().mfx_DebugVisual)
	{
		if (effectId != InvalidEffectId)
		{
			m_pVisualDebug->AddLastSearchHint(effectId, customName, surfaceIndex2);
		}
		else
		{
			GameWarning("Could not find a valid effect at row %i and column %i of libs/materialeffects/materialeffects.xml", idx1, idx2 );
		}
	}

	return effectId;
#else
	return InternalGetEffectId(idx1, idx2);
#endif

}

// Get the cell contents that these parameters equate to
TMFXEffectId CMaterialEffects::GetEffectId(IEntityClass* pEntityClass, i32 surfaceIndex2)
{
	if (!CMaterialEffectsCVars::Get().mfx_Enable)
		return InvalidEffectId;

	// Map material IDs to effect indexes
	const TIndex idx1 = stl::find_in_map(m_ptrToMatrixEntryMap, pEntityClass, 0);
	const TIndex idx2 = SurfaceIdToMatrixEntry(surfaceIndex2);

#ifdef MATERIAL_EFFECTS_DEBUG
	TMFXEffectId effectId = InternalGetEffectId(idx1, idx2);

	if (CMaterialEffectsCVars::Get().mfx_DebugVisual)
	{
		if (effectId != InvalidEffectId)
		{
			m_pVisualDebug->AddLastSearchHint(effectId, pEntityClass, surfaceIndex2);
		}
		else
		{
			GameWarning("Could not find a valid effect at row %i and column %i of libs/materialeffects/materialeffects.xml", idx1, idx2);
		}
	}

	return effectId;
#else
	return InternalGetEffectId(idx1, idx2);
#endif

}

SMFXResourceListPtr CMaterialEffects::GetResources(TMFXEffectId effectId) const
{
	SMFXResourceListPtr pResourceList = SMFXResourceList::Create();

	TMFXContainerPtr pEffectContainer = InternalGetEffect(effectId);
	if (pEffectContainer)
	{
		pEffectContainer->GetResources(*pResourceList);
	}

	return pResourceList;
}

void CMaterialEffects::TimedEffect(TMFXContainerPtr pEffectContainer, const SMFXRunTimeEffectParams& params)
{
	if (!m_bUpdateMode)
		return;

	m_delayedEffects.push_back(SDelayedEffect(pEffectContainer, params));
}

void CMaterialEffects::SetUpdateMode(bool bUpdate)
{
	if (!bUpdate)
	{
		m_delayedEffects.clear();
		m_pMaterialFGUpr->Reset(false);
	}

	m_bUpdateMode = bUpdate;
}

void CMaterialEffects::FullReload()
{
	Reset(true);

	LoadFXLibraries();
	LoadSpreadSheet();
	LoadFlowGraphLibs();
}

void CMaterialEffects::Update(float frameTime)
{
	SetUpdateMode(true);
	std::vector<SDelayedEffect>::iterator it = m_delayedEffects.begin();
	std::vector<SDelayedEffect>::iterator next = it;
	while (it != m_delayedEffects.end())
	{
		++next;
		SDelayedEffect& cur = *it;
		cur.m_delay -= frameTime;
		if (cur.m_delay <= 0.0f)
		{
			cur.m_pEffectContainer->Execute(cur.m_effectRuntimeParams);
			next = m_delayedEffects.erase(it);
		}
		it = next;
	}

#ifdef MATERIAL_EFFECTS_DEBUG
	if (CMaterialEffectsCVars::Get().mfx_DebugVisual)
	{
		m_pVisualDebug->Update(*this, frameTime);
	}
#endif
}

void CMaterialEffects::NotifyFGHudEffectEnd(IFlowGraphPtr pFG)
{
	if (m_pMaterialFGUpr)
		m_pMaterialFGUpr->EndFGEffect(pFG);
}

void CMaterialEffects::Reset(bool bCleanup)
{
	// make sure all pre load data has been propperly released to not have any
	// dangling pointers are for example the materials itself have been flushed
	for (TMFXEffectId id = 0; id < m_effectContainers.size(); ++id)
	{
		if (m_effectContainers[id])
		{
			m_effectContainers[id]->ReleasePreLoadAssets();
		}
	}

	if (m_pMaterialFGUpr)
		m_pMaterialFGUpr->Reset(bCleanup);

	if (bCleanup)
	{
		stl::free_container(m_mfxLibraries);
		stl::free_container(m_delayedEffects);
		stl::free_container(m_effectContainers);
		stl::free_container(m_customConvert);
		stl::free_container(m_surfaceIdToMatrixEntry);
		stl::free_container(m_ptrToMatrixEntryMap);
		m_surfacesLookupTable.Free();
		m_bDataInitialized = false;

		SMFXResourceList::FreePool();
		SMFXFlowGraphListNode::FreePool();
		SMFXDecalListNode::FreePool();
		SMFXParticleListNode::FreePool();
		SMFXAudioListNode::FreePool();
		SMFXForceFeedbackListNode::FreePool();
	}
}

void CMaterialEffects::ClearDelayedEffects()
{
	m_delayedEffects.resize(0);
}

void CMaterialEffects::Serialize(TSerialize ser)
{
	if (m_pMaterialFGUpr && CMaterialEffectsCVars::Get().mfx_SerializeFGEffects != 0)
		m_pMaterialFGUpr->Serialize(ser);
}

void CMaterialEffects::GetMemoryUsage(IDrxSizer* s) const
{
	SIZER_SUBCOMPONENT_NAME(s, "MaterialEffects");
	s->AddObject(this, sizeof(*this));
	s->AddObject(m_pMaterialFGUpr);

	{
		SIZER_SUBCOMPONENT_NAME(s, "libs");
		s->AddObject(m_mfxLibraries);
	}
	{
		SIZER_SUBCOMPONENT_NAME(s, "convert");
		s->AddObject(m_customConvert);
		s->AddObject(m_surfaceIdToMatrixEntry);
		s->AddObject(m_ptrToMatrixEntryMap);
	}
	{
		SIZER_SUBCOMPONENT_NAME(s, "lookup");
		s->AddObject(m_effectContainers); // the effects themselves already accounted in "libs"
		s->AddObject(m_surfacesLookupTable);
	}
	{
		SIZER_SUBCOMPONENT_NAME(s, "playing");
		s->AddObject(m_delayedEffects); // the effects themselves already accounted in "libs"
		//s->AddObject((ukk )&CMFXRandomEffect::GetMemoryUsage, CMFXRandomEffect::GetMemoryUsage());
	}
}

static float NormalizeMass(float fMass)
{
	float massMin = 0.0f;
	float massMax = 500.0f;
	float paramMin = 0.0f;
	float paramMax = 1.0f / 3.0f;

	// tiny - bullets
	if (fMass <= 0.1f)
	{
		// small
		massMin = 0.0f;
		massMax = 0.1f;
		paramMin = 0.0f;
		paramMax = 1.0f;
	}
	else if (fMass < 20.0f)
	{
		// small
		massMin = 0.0f;
		massMax = 20.0f;
		paramMin = 0.0f;
		paramMax = 1.0f / 3.0f;
	}
	else if (fMass < 200.0f)
	{
		// medium
		massMin = 20.0f;
		massMax = 200.0f;
		paramMin = 1.0f / 3.0f;
		paramMax = 2.0f / 3.0f;
	}
	else
	{
		// ultra large
		massMin = 200.0f;
		massMax = 2000.0f;
		paramMin = 2.0f / 3.0f;
		paramMax = 1.0f;
	}

	const float p = min(1.0f, (fMass - massMin) / (massMax - massMin));
	const float finalParam = paramMin + (p * (paramMax - paramMin));
	return finalParam;
}

bool CMaterialEffects::PlayBreakageEffect(ISurfaceType* pSurfaceType, tukk breakageType, const SMFXBreakageParams& breakageParams)
{
	if (pSurfaceType == 0)
		return false;

	DrxFixedStringT<128> fxName("Breakage:");
	fxName += breakageType;
	TMFXEffectId effectId = this->GetEffectId(fxName.c_str(), pSurfaceType->GetId());
	if (effectId == InvalidEffectId)
		return false;

	// only play sound at the moment
	SMFXRunTimeEffectParams params;
	params.playflags = eMFXPF_Audio;

	// if hitpos is set, use it
	// otherwise use matrix (hopefully been set or 0,0,0)
	if (breakageParams.CheckFlag(SMFXBreakageParams::eBRF_HitPos) && breakageParams.GetHitPos().IsZero() == false)
		params.pos = breakageParams.GetHitPos();
	else
		params.pos = breakageParams.GetMatrix().GetTranslation();

	//params.soundSemantic = eSoundSemantic_Physics_General;

	const Vec3& hitImpulse = breakageParams.GetHitImpulse();
	const float strength = hitImpulse.GetLengthFast();
	params.AddAudioRtpc("strength", strength);
	const float mass = NormalizeMass(breakageParams.GetMass());
	params.AddAudioRtpc("mass", mass);

	if (CMaterialEffectsCVars::Get().mfx_Debug & 2)
	{
		TMFXContainerPtr pEffectContainer = InternalGetEffect(effectId);
		if (pEffectContainer != NULL)
		{
			DrxLogAlways("[MFX]: %s:%s FX=%s:%s Pos=%f,%f,%f NormMass=%f  F=%f Imp=%f,%f,%f  RealMass=%f Vel=%f,%f,%f",
			             breakageType, pSurfaceType->GetName(), pEffectContainer->GetParams().libraryName.c_str(), pEffectContainer->GetParams().name.c_str(),
			             params.pos[0], params.pos[1], params.pos[2],
			             mass,
			             strength,
			             breakageParams.GetHitImpulse()[0],
			             breakageParams.GetHitImpulse()[1],
			             breakageParams.GetHitImpulse()[2],
			             breakageParams.GetMass(),
			             breakageParams.GetVelocity()[0],
			             breakageParams.GetVelocity()[1],
			             breakageParams.GetVelocity()[2]
			             );
		}
	}

	/*
	   if (breakageParams.GetMass() == 0.0f)
	   {
	   i32 a = 0;
	   }
	 */

	const bool bSuccess = ExecuteEffect(effectId, params);

	return bSuccess;
}

void CMaterialEffects::CompleteInit()
{
	LOADING_TIME_PROFILE_SECTION

	if (m_bDataInitialized)
		return;

	LoadFXLibraries();
	LoadSpreadSheet();
	LoadFlowGraphLibs();
}

i32 CMaterialEffects::GetDefaultCanopyIndex()
{
#ifdef MATERIAL_EFFECTS_DEBUG
	if (m_defaultSurfaceId == m_canopySurfaceId)
	{
		GameWarning("[MFX] CMaterialEffects::GetDefaultCanopyIndex returning default - called before MFX loaded");
	}
#endif

	return m_canopySurfaceId;
}

void CMaterialEffects::ReloadMatFXFlowGraphs()
{
	m_pMaterialFGUpr->ReloadFlowGraphs();
}

size_t CMaterialEffects::GetMatFXFlowGraphCount() const
{
	return m_pMaterialFGUpr->GetFlowGraphCount();
}

IFlowGraphPtr CMaterialEffects::GetMatFXFlowGraph(i32 index, string* pFileName /*= NULL*/) const
{
	return m_pMaterialFGUpr->GetFlowGraph(index, pFileName);
}

IFlowGraphPtr CMaterialEffects::LoadNewMatFXFlowGraph(const string& filename)
{
	IFlowGraphPtr res;
	m_pMaterialFGUpr->LoadFG(filename, &res);
	return res;
}

void CMaterialEffects::EnumerateEffectNames(EnumerateMaterialEffectsDataCallback& callback, tukk szLibraryName) const
{
	TFXLibrariesMap::const_iterator pLibraryIter = m_mfxLibraries.find(szLibraryName);
	if (pLibraryIter != m_mfxLibraries.end())
	{
		CMFXLibrary::TEffectNames effectNames;
		pLibraryIter->second.GetEffectNames(effectNames);
		for (const string& curEntry : effectNames)
		{
			callback(curEntry);
		}
	}
}

void CMaterialEffects::EnumerateLibraryNames(EnumerateMaterialEffectsDataCallback& callback) const
{
	for (const auto& entry : m_mfxLibraries)
	{
		callback(entry.first.c_str());
	}
}
void CMaterialEffects::UnloadFXLibrariesWithPrefix(tukk szName)
{
	for (TFXLibrariesMap::iterator library = m_mfxLibraries.begin(); library != m_mfxLibraries.end(); )
	{
		if (library->first.find(szName) != string::npos)
		{
			library = m_mfxLibraries.erase(library);
		}
		else
		{
			++library;
		}
	}
}

void CMaterialEffects::LoadFXLibraryFromXMLInMemory(tukk szName, XmlNodeRef root)
{
	MEMSTAT_CONTEXT_FMT(EMemStatContextTypes::MSC_Other, 0, "FX Library XML (%s)", szName);

	const string libName = szName;

	XmlNodeRef libraryRootNode = root;
	if (libraryRootNode != 0)
	{
		TFXLibrariesMap::iterator libIter = m_mfxLibraries.find(libName);
		if (libIter != m_mfxLibraries.end())
		{
			m_mfxLibraries.erase(libIter);
		}

		std::pair<TFXLibrariesMap::iterator, bool> iterPair = m_mfxLibraries.insert(TFXLibrariesMap::value_type(libName, CMFXLibrary()));
		assert(iterPair.second == true);
		libIter = iterPair.first;
		assert(libIter != m_mfxLibraries.end());

		const TMFXNameId& libraryNameId = libIter->first; // uses DrxString's ref-count feature
		CMFXLibrary& mfxLibrary = libIter->second;

		CMFXLibrary::SLoadingEnvironment libraryLoadingEnvironment(libraryNameId, libraryRootNode, m_effectContainers);
		mfxLibrary.LoadFromXml(libraryLoadingEnvironment);
	}
	else
	{
		GameWarning("[MatFX]: Failed to load library %s", libName.c_str());
	}
}
