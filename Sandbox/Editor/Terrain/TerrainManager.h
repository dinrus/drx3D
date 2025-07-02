// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef TerrainManager_h__
#define TerrainManager_h__

#pragma once

//////////////////////////////////////////////////////////////////////////
// Dependencies
#include "Terrain/Heightmap.h"
#include "DocMultiArchive.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class CLayer;
class CSurfaceType;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Class
class SANDBOX_API CTerrainManager
{
public:
	CTerrainManager();
	virtual ~CTerrainManager();

	//////////////////////////////////////////////////////////////////////////
	// Surface Types.
	//////////////////////////////////////////////////////////////////////////
	// Returns:
	//   can be 0 if the id does not exit
	CSurfaceType* GetSurfaceTypePtr(i32 i) const { if (i >= 0 && i < m_surfaceTypes.size()) return m_surfaceTypes[i]; return 0; }
	i32           GetSurfaceTypeCount() const    { return m_surfaceTypes.size(); }
	//! Find surface type by name, return -1 if name not found.
	i32           FindSurfaceType(const string& name);
	void          RemoveSurfaceType(CSurfaceType* pSrfType);
	i32           AddSurfaceType(CSurfaceType* srf);
	void          RemoveAllSurfaceTypes();
	void          SerializeSurfaceTypes(CXmlArchive& xmlAr, bool bUpdateEngineTerrain = true);
	void          ConsolidateSurfaceTypes();

	/** Put surface types from Editor to game.
	 */
	void ReloadSurfaceTypes(bool bUpdateEngineTerrain = true, bool bUpdateHeightmap = true);

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Layers
	i32     GetLayerCount() const     { return m_layers.size(); };
	CLayer* GetLayer(i32 layer) const { return m_layers[layer]; };
	void    SelectLayer(i32 layerIndex);
	i32     GetSelectedLayerIndex();
	CLayer* GetSelectedLayer();
	//! Find layer by name.
	CLayer* FindLayer(tukk sLayerName) const;
	CLayer* FindLayerByLayerId(u32k dwLayerId) const;
	void    SwapLayers(i32 layer1, i32 layer2);
	void    AddLayer(CLayer* layer);
	void    RemoveLayer(CLayer* layer);
	void    InvalidateLayers();
	void    ClearLayers();
	void    SerializeLayerSettings(CXmlArchive& xmlAr);
	void    MarkUsedLayerIds(bool bFree[256]) const;

	void    CreateDefaultLayer();

	// slow
	// Returns:
	//   0xffffffff if the method failed (internal error)
	u32 GetDetailIdLayerFromLayerId(u32k dwLayerId);

	// needed to convert from old layer structure to the new one
	bool ConvertLayersToRGBLayer();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// Heightmap
	CHeightmap* GetHeightmap() { return &m_heightmap; };
	CRGBLayer*  GetRGBLayer();

	void        SetTerrainSize(i32 resolution, float unitSize);
	void        ResetHeightMap();
	bool        WouldHeightmapSaveSucceed();

	void        Save(bool bBackup = false);
	bool        Load();

	void        SaveTexture(bool bBackup = false);
	bool        LoadTexture();

	void        SerializeTerrain(TDocMultiArchive& arrXmlAr);
	void        SerializeTerrain(CXmlArchive& xmlAr);
	void        SerializeTexture(CXmlArchive& xmlAr);

	void        SetModified(i32 x1 = 0, i32 y1 = 0, i32 x2 = 0, i32 y2 = 0);

	void        GetTerrainMemoryUsage(IDrxSizer* pSizer);
	//////////////////////////////////////////////////////////////////////////

	// Get the number of terrain data files in the level data folder.
	// \sa GetIEditorImpl()->GetLevelDataFolder(). 
	i32         GetDataFilesCount() const;
	tukk GetDataFilename(i32 i) const;

	CDrxSignal<void(void)>    signalLayersChanged;
	CDrxSignal<void(CLayer*)> signalSelectedLayerChanged;

protected:
	std::vector<_smart_ptr<CSurfaceType>> m_surfaceTypes;
	std::vector<CLayer*>                  m_layers;
	CHeightmap                            m_heightmap;
};

#endif // TerrainManager_h__

