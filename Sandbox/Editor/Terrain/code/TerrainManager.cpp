// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <StdAfx.h>
#include "TerrainManager.h"

//////////////////////////////////////////////////////////////////////////
#include "SurfaceType.h"
#include "Layer.h"
#include "TerrainTexGen.h"
#include "Util/AutoLogTime.h"

#include "QT/Widgets/QWaitProgress.h"

#include "DrxEditDoc.h"

namespace {
tukk kHeightmapFile = "Heightmap.dat";
tukk kHeightmapFileOld = "Heightmap.xml";    //TODO: Remove support after January 2012 (support old extension at least for convertion levels time)
tukk kTerrainTextureFile = "TerrainTexture.xml";
};

//////////////////////////////////////////////////////////////////////////
// CTerrainManager

//////////////////////////////////////////////////////////////////////////
CTerrainManager::CTerrainManager()
{
}

//////////////////////////////////////////////////////////////////////////
CTerrainManager::~CTerrainManager()
{
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::RemoveSurfaceType(CSurfaceType* pSurfaceType)
{
	for (i32 i = 0; i < m_surfaceTypes.size(); i++)
	{
		if (m_surfaceTypes[i] == pSurfaceType)
		{
			m_surfaceTypes.erase(m_surfaceTypes.begin() + i);
			break;
		}
	}
	ConsolidateSurfaceTypes();
}

//////////////////////////////////////////////////////////////////////////
i32 CTerrainManager::AddSurfaceType(CSurfaceType* srf)
{
	GetIEditorImpl()->SetModifiedFlag(TRUE);
	m_surfaceTypes.push_back(srf);

	return m_surfaceTypes.size() - 1;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::RemoveAllSurfaceTypes()
{
	LOADING_TIME_PROFILE_SECTION;
	GetIEditorImpl()->SetModifiedFlag(TRUE);
	m_surfaceTypes.clear();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SerializeSurfaceTypes(CXmlArchive& xmlAr, bool bUpdateEngineTerrain)
{
	if (xmlAr.bLoading)
	{
		// Clear old layers
		RemoveAllSurfaceTypes();

		// Load the layer count
		XmlNodeRef node = xmlAr.root->findChild("SurfaceTypes");
		if (!node)
			return;

		// Read all node
		for (i32 i = 0; i < node->getChildCount(); i++)
		{
			CXmlArchive ar(xmlAr);
			ar.root = node->getChild(i);
			CSurfaceType* sf = new CSurfaceType;
			sf->Serialize(ar);
			AddSurfaceType(sf);

			if (sf->GetSurfaceTypeID() == CLayer::e_undefined)  // For older levels.
			{
				sf->AssignUnusedSurfaceTypeID();
			}
		}
		ReloadSurfaceTypes(bUpdateEngineTerrain);
	}
	else
	{
		// Storing
		CLogFile::WriteLine("Storing surface types...");

		// Save the layer count

		XmlNodeRef node = xmlAr.root->newChild("SurfaceTypes");

		// Write all surface types.
		for (i32 i = 0; i < m_surfaceTypes.size(); i++)
		{
			CXmlArchive ar(xmlAr);
			ar.root = node->newChild("SurfaceType");
			m_surfaceTypes[i]->Serialize(ar);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::ConsolidateSurfaceTypes()
{
	// We must consolidate the IDs after removing
	for (size_t i = 0; i < m_surfaceTypes.size(); ++i)
	{
		i32 id = i < CLayer::e_undefined ? i : CLayer::e_undefined;
		m_surfaceTypes[i]->SetSurfaceTypeID(id);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::ReloadSurfaceTypes(bool bUpdateEngineTerrain, bool bUpdateHeightmap)
{
	LOADING_TIME_PROFILE_SECTION(gEnv->pSystem);

	CXmlArchive ar;
	XmlNodeRef node = XmlHelpers::CreateXmlNode("SurfaceTypes_Editor");
	// Write all surface types.
	for (i32 i = 0; i < GetSurfaceTypeCount(); i++)
	{
		ar.root = node->newChild("SurfaceType");
		GetSurfaceTypePtr(i)->Serialize(ar);
	}

	gEnv->p3DEngine->LoadTerrainSurfacesFromXML(node, bUpdateEngineTerrain);

	if (bUpdateHeightmap && gEnv->p3DEngine->GetITerrain())
	{
		m_heightmap.UpdateEngineTerrain(false, false);
	}

	signalLayersChanged();
}

//////////////////////////////////////////////////////////////////////////
i32 CTerrainManager::FindSurfaceType(const string& name)
{
	for (i32 i = 0; i < m_surfaceTypes.size(); i++)
	{
		if (stricmp(m_surfaceTypes[i]->GetName(), name) == 0)
			return i;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
CLayer* CTerrainManager::FindLayer(tukk sLayerName) const
{
	for (i32 i = 0; i < GetLayerCount(); i++)
	{
		CLayer* layer = GetLayer(i);
		if (strcmp(layer->GetLayerName(), sLayerName) == 0)
			return layer;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CLayer* CTerrainManager::FindLayerByLayerId(u32k dwLayerId) const
{
	for (i32 i = 0; i < GetLayerCount(); i++)
	{
		CLayer* layer = GetLayer(i);
		if (layer->GetCurrentLayerId() == dwLayerId)
			return layer;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::RemoveLayer(CLayer* layer)
{
	SelectLayer(-1);

	if (layer && layer->GetCurrentLayerId() != CLayer::e_undefined)
	{
		u32 id = layer->GetCurrentLayerId();

		if (id != CLayer::e_undefined)
			m_heightmap.EraseLayerID(id);
	}

	if (layer)
	{
		delete layer;
		m_layers.erase(std::remove(m_layers.begin(), m_layers.end(), layer), m_layers.end());
	}

	signalLayersChanged();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SwapLayers(i32 layer1, i32 layer2)
{
	assert(layer1 >= 0 && layer1 < m_layers.size());
	assert(layer2 >= 0 && layer2 < m_layers.size());
	std::swap(m_layers[layer1], m_layers[layer2]);

	// If necessary, also swap the surface types (their order is important when using masked surface type transitions)
	i32 st1 = m_layers[layer1]->GetEngineSurfaceTypeId();
	i32 st2 = m_layers[layer2]->GetEngineSurfaceTypeId();
	CSurfaceType* pSurfaceType1 = GetSurfaceTypePtr(st1);
	CSurfaceType* pSurfaceType2 = GetSurfaceTypePtr(st2);
	if (pSurfaceType1 && pSurfaceType2 && sgn(st1 - st2) == -sgn(layer1 - layer2))
	{
		CLayer TempLayer;
		TempLayer.SetSurfaceType(pSurfaceType1); // Temporarily holds the surface type so that it does not get deleted
		m_layers[layer1]->SetSurfaceType(pSurfaceType2);
		m_layers[layer2]->SetSurfaceType(pSurfaceType1);
		TempLayer.SetSurfaceType(NULL);
		CSurfaceType& surfaceType1 = *m_surfaceTypes[st1];
		CSurfaceType& surfaceType2 = *m_surfaceTypes[st2];
		std::swap(surfaceType1, surfaceType2);
		m_heightmap.UpdateEngineTerrain();
	}

	signalLayersChanged();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::InvalidateLayers()
{
	////////////////////////////////////////////////////////////////////////
	// Set the update needed flag for all layer
	////////////////////////////////////////////////////////////////////////
	for (i32 i = 0; i < GetLayerCount(); ++i)
		GetLayer(i)->InvalidateMask();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::ClearLayers()
{
	LOADING_TIME_PROFILE_SECTION;
	SelectLayer(-1);
	////////////////////////////////////////////////////////////////////////
	// Clear all texture layers
	////////////////////////////////////////////////////////////////////////
	for (i32 i = 0; i < GetLayerCount(); ++i)
		delete GetLayer(i);

	m_layers.clear();

	signalLayersChanged();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SerializeLayerSettings(CXmlArchive& xmlAr)
{
	////////////////////////////////////////////////////////////////////////
	// Load or restore the layer settings from an XML
	////////////////////////////////////////////////////////////////////////
	if (xmlAr.bLoading)
	{
		// Loading
		i32 selected_layer = GetSelectedLayerIndex();
		CLogFile::WriteLine("Loading layer settings...");
		CWaitProgress progress(_T("Loading Terrain Layers"));

		// Clear old layers
		ClearLayers();

		// Load the layer count
		XmlNodeRef layers = xmlAr.root->findChild("Layers");
		if (!layers)
			return;

		// Read all layers
		i32 numLayers = layers->getChildCount();
		for (i32 i = 0; i < numLayers; i++)
		{
			progress.Step(100 * i / numLayers);
			// Create a new layer
			m_layers.push_back(new CLayer);

			CXmlArchive ar(xmlAr);
			ar.root = layers->getChild(i);
			// Fill the layer with the data
			GetLayer(i)->Serialize(ar);

			DrxLog("  loaded editor layer %d  name='%s' LayerID=%d", i, GetLayer(i)->GetLayerName(), GetLayer(i)->GetCurrentLayerId());
		}

		// If surface type ids are unassigned, assign them.
		for (i32 i = 0; i < GetSurfaceTypeCount(); i++)
		{
			if (GetSurfaceTypePtr(i)->GetSurfaceTypeID() >= CLayer::e_undefined)
				GetSurfaceTypePtr(i)->AssignUnusedSurfaceTypeID();
		}

		signalLayersChanged();

		SelectLayer(selected_layer);
	}
	else
	{
		// Storing

		CLogFile::WriteLine("Storing layer settings...");

		// Save the layer count

		XmlNodeRef layers = xmlAr.root->newChild("Layers");

		// Write all layers
		for (i32 i = 0; i < GetLayerCount(); i++)
		{
			CXmlArchive ar(xmlAr);
			ar.root = layers->newChild("Layer");
			;
			GetLayer(i)->Serialize(ar);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
u32 CTerrainManager::GetDetailIdLayerFromLayerId(u32k dwLayerId)
{
	for (i32 i = 0, num = (i32)m_layers.size(); i < num; i++)
	{
		CLayer& rLayer = *(m_layers[i]);

		if (dwLayerId == rLayer.GetOrRequestLayerId())
		{
			return rLayer.GetEngineSurfaceTypeId();
		}
	}

	// recreate referenced layer
	if (!GetIEditorImpl()->GetSystem()->GetI3DEngine()->GetITerrain())   // only if terrain loaded successfully
	{
		string no;

		no.Format("%d", dwLayerId);

		CLayer* pNewLayer = new CLayer;

		pNewLayer->SetLayerName(string("LAYER_") + no);
		pNewLayer->SetLayerId(dwLayerId);

		AddLayer(pNewLayer);
	}

	return CLayer::e_undefined;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::MarkUsedLayerIds(bool bFree[CLayer::e_undefined]) const
{
	std::vector<CLayer*>::const_iterator it;

	for (it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CLayer* pLayer = *it;

		u32k id = pLayer->GetCurrentLayerId();

		if (id < CLayer::e_undefined)
			bFree[id] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::CreateDefaultLayer()
{
	// Create default layer.
	CLayer* layer = new CLayer;
	AddLayer(layer);
	layer->SetLayerName("Default");
	layer->LoadTexture("%ENGINE%/engineassets/textures/grey.dds");
	layer->SetLayerId(0);

	// Create default surface type.
	CSurfaceType* sfType = new CSurfaceType;
	sfType->SetName("%ENGINE%/EngineAssets/Materials/material_terrain_default");
	u32 dwDetailLayerId = AddSurfaceType(sfType);
	sfType->SetMaterial("%ENGINE%/EngineAssets/Materials/material_terrain_default");
	sfType->AssignUnusedSurfaceTypeID();

	layer->SetSurfaceType(sfType);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SerializeTerrain(TDocMultiArchive& arrXmlAr)
{
	bool bLoading = IsLoadingXmlArArray(arrXmlAr);

	if (bLoading)
	{
		m_heightmap.Serialize((*arrXmlAr[DMAS_GENERAL]));

		// Surface Types ///////////////////////////////////////////////////////
		{
			CAutoLogTime logtime("Loading Surface Types");
			SerializeSurfaceTypes((*arrXmlAr[DMAS_TERRAIN_LAYERS]));
		}

		// Terrain texture /////////////////////////////////////////////////////
		{
			CAutoLogTime logtime("Loading Terrain Layers Info");
			SerializeLayerSettings((*arrXmlAr[DMAS_TERRAIN_LAYERS]));
		}

		{
			CAutoLogTime logtime("Load Terrain");

			m_heightmap.SerializeTerrain((*arrXmlAr[DMAS_GENERAL]));

			{
				XmlNodeRef layers = (*arrXmlAr[DMAS_GENERAL]).root->findChild("Layers");
				if (layers)
				{
					i32 numLayers = layers->getChildCount();
					for (i32 i = 0; i < numLayers; i++)
					{
						GetLayer(i)->Update3dengineInfo();
					}
				}
			}
		}

		if (!m_heightmap.IsAllocated())
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Heightmap data wasn't properly loaded. The file is missing or corrupted. Using this level is not recommended. Update level data from your backup.");
		}
		else
		{
			CAutoLogTime logtime("Process RGB Terrain Layers");
			ConvertLayersToRGBLayer();
		}
	}
	else
	{
		if (arrXmlAr[DMAS_GENERAL] != NULL)
		{
			// save terrain heightmap data
			m_heightmap.Serialize((*arrXmlAr[DMAS_GENERAL]));
			m_heightmap.SerializeTerrain((*arrXmlAr[DMAS_GENERAL]));
		}

		if (arrXmlAr[DMAS_TERRAIN_LAYERS] != NULL)
		{
			// Surface Types
			SerializeSurfaceTypes((*arrXmlAr[DMAS_TERRAIN_LAYERS]));
			SerializeLayerSettings((*arrXmlAr[DMAS_TERRAIN_LAYERS]));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SerializeTerrain(CXmlArchive& xmlAr)
{
	if (xmlAr.bLoading)
	{
		m_heightmap.Serialize(xmlAr);

		// Surface Types ///////////////////////////////////////////////////////
		{
			CAutoLogTime logtime("Loading Surface Types");
			SerializeSurfaceTypes(xmlAr);
		}

		// Terrain texture /////////////////////////////////////////////////////
		{
			CAutoLogTime logtime("Loading Terrain Layers Info");
			SerializeLayerSettings(xmlAr);
		}

		{
			CAutoLogTime logtime("Load Terrain");
			m_heightmap.SerializeTerrain(xmlAr);

			{
				XmlNodeRef layers = xmlAr.root->findChild("Layers");
				if (layers)
				{
					i32 numLayers = layers->getChildCount();
					for (i32 i = 0; i < numLayers; i++)
					{
						GetLayer(i)->Update3dengineInfo();
					}
				}
			}
		}

		if (!m_heightmap.IsAllocated())
		{
			DrxWarning(VALIDATOR_MODULE_EDITOR, VALIDATOR_ERROR, "Heightmap data wasn't properly loaded. The file is missing or corrupted. Using this level is not recommended. Update level data from your backup.");
		}
		else
		{
			CAutoLogTime logtime("Process RGB Terrain Layers");
			ConvertLayersToRGBLayer();
		}
	}
	else
	{
		// save terrain heightmap data
		m_heightmap.Serialize(xmlAr);

		m_heightmap.SerializeTerrain(xmlAr);

		// Surface Types
		SerializeSurfaceTypes(xmlAr);

		SerializeLayerSettings(xmlAr);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SerializeTexture(CXmlArchive& xmlAr)
{
	GetRGBLayer()->Serialize(xmlAr.root, xmlAr.bLoading);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::GetTerrainMemoryUsage(IDrxSizer* pSizer)
{
	{
		SIZER_COMPONENT_NAME(pSizer, "Layers");

		std::vector<CLayer*>::iterator it;

		for (it = m_layers.begin(); it != m_layers.end(); ++it)
		{
			CLayer* pLayer = *it;

			pLayer->GetMemoryUsage(pSizer);
		}
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "CHeightmap");

		m_heightmap.GetMemoryUsage(pSizer);
	}

	{
		SIZER_COMPONENT_NAME(pSizer, "RGBLayer");

		GetRGBLayer()->GetMemoryUsage(pSizer);
	}
}

i32 CTerrainManager::GetDataFilesCount() const
{
	return 2;
}

tukk CTerrainManager::GetDataFilename(i32 i) const
{
	switch (i)
	{
	case 0:
		return kHeightmapFile;
	case 1:
		return kTerrainTextureFile;
	default:
		return nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainManager::ConvertLayersToRGBLayer()
{
	bool bConvertNeeded = !GetRGBLayer()->IsAllocated();

	if (!bConvertNeeded)
		return false;

	std::vector<CLayer*>::iterator it;

	u32 dwTexResolution = 0;

	for (it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CLayer* pLayer = *it;

		dwTexResolution = max(dwTexResolution, (u32)pLayer->GetMaskResolution());
		/*
		   if(pLayer->GetMask().IsValid())
		   {
		   dwTexResolution=pLayer->GetMask().GetWidth();
		   bLayersStillHaveAMask=true;
		   break;
		   }
		 */
	}

	if (QDialogButtonBox::StandardButton::No == CQuestionDialog::SQuestion(QObject::tr(""), QObject::tr("Convert LayeredTerrainTexture to RGBTerrainTexture?"), QDialogButtonBox::StandardButton::Yes | QDialogButtonBox::StandardButton::No))
	{
		return false;
	}

	if (QDialogButtonBox::StandardButton::Yes == CQuestionDialog::SQuestion(QObject::tr(""), QObject::tr("Double the resolution of the existing data?"), QDialogButtonBox::StandardButton::Yes | QDialogButtonBox::StandardButton::No))
		dwTexResolution *= 2;

	// extract RGB texture
	{
		CTerrainLayerTexGen texGen;

		texGen.Init(dwTexResolution);

		SSectorInfo si;

		GetIEditorImpl()->GetHeightmap()->GetSectorsInfo(si);

		u32 dwNumSectors = si.numSectors;

		u32 dwSectorTexResolution = 512;
		u32 dwSectorResolution = dwTexResolution / dwNumSectors;
		u32 dwNumTexSectors = dwTexResolution / dwSectorTexResolution;

		// no error check because this is allocating only very few bytes
		GetRGBLayer()->AllocateTiles(dwNumTexSectors, dwNumTexSectors, dwSectorTexResolution);  // dwTexResolution x dwTexResolution;

		CImageEx sectorDiffuseImage;

		if (!sectorDiffuseImage.Allocate(dwSectorResolution, dwSectorResolution))
			return false;

		i32 flags = ETTG_QUIET | ETTG_NO_TERRAIN_SHADOWS | ETTG_ABGR;

		for (i32 y = 0; y < dwNumSectors; y++)
			for (i32 x = 0; x < dwNumSectors; x++)
			{
				CPoint sector(x, y);

				CRect rect(0, 0, sectorDiffuseImage.GetWidth(), sectorDiffuseImage.GetHeight());

				if (!texGen.GenerateSectorTexture(sector, rect, flags, sectorDiffuseImage))
					return false;     // maybe this should be handled better

				// M.M. to take a look at the result of the calculation
				/*char str[80];
				   drx_sprintf(str,"c:\\temp\\tile%d_%d.bmp",x,y);
				   CImageUtil::SaveBitmap(str,sectorDiffuseImage);
				 */
				GetRGBLayer()->SetSubImageRGBLayer(x * dwSectorResolution, y * dwSectorResolution, sectorDiffuseImage);
			}
	}

	// extract Detail LayerId info
	GetIEditorImpl()->GetHeightmap()->CalcSurfaceTypes();

	// we no longer need the layer mask data
	for (it = m_layers.begin(); it != m_layers.end(); ++it)
	{
		CLayer* pLayer = *it;

		pLayer->ReleaseMask();
	}

	GetIEditorImpl()->GetHeightmap()->UpdateEngineTerrain(false);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SetTerrainSize(i32 resolution, float unitSize)
{
	m_heightmap.Resize(resolution, resolution, unitSize);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::ResetHeightMap()
{
	////////////////////////////////////////////////////////////////////////
	// Reset Heightmap
	////////////////////////////////////////////////////////////////////////
	m_heightmap.ClearModSectors();
	m_heightmap.SetWaterLevel(16); // Default water level.
	SetTerrainSize(1024, 2);
	m_heightmap.SetMaxHeight(1024);
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainManager::WouldHeightmapSaveSucceed()
{
	return GetRGBLayer()->WouldSaveSucceed();
}

//////////////////////////////////////////////////////////////////////////
CRGBLayer* CTerrainManager::GetRGBLayer()
{
	return m_heightmap.GetRGBLayer();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::Save(bool bBackup)
{
	LOADING_TIME_PROFILE_SECTION;
	CTempFileHelper helper(GetIEditorImpl()->GetLevelDataFolder() + kHeightmapFile);

	CXmlArchive xmlAr;
	SerializeTerrain(xmlAr);
	xmlAr.Save(helper.GetTempFilePath());

	helper.UpdateFile(bBackup);
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainManager::Load()
{
	LOADING_TIME_PROFILE_SECTION;
	string filename = GetIEditorImpl()->GetLevelDataFolder() + kHeightmapFile;
	CXmlArchive xmlAr;
	xmlAr.bLoading = true;
	if (!xmlAr.Load(filename))
	{
		string filename = GetIEditorImpl()->GetLevelDataFolder() + kHeightmapFileOld;
		if (!xmlAr.Load(filename))
		{
			return false;
		}
	}

	SerializeTerrain(xmlAr);
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SaveTexture(bool bBackup)
{
	LOADING_TIME_PROFILE_SECTION;
	CTempFileHelper helper(GetIEditorImpl()->GetLevelDataFolder() + kTerrainTextureFile);

	XmlNodeRef root;
	GetRGBLayer()->Serialize(root, false);
	if (root)
		root->saveToFile(helper.GetTempFilePath());

	helper.UpdateFile(bBackup);
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainManager::LoadTexture()
{
	string filename = GetIEditorImpl()->GetLevelDataFolder() + kTerrainTextureFile;
	XmlNodeRef root = XmlHelpers::LoadXmlFromFile(filename);
	if (root)
	{
		GetRGBLayer()->Serialize(root, true);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainManager::SetModified(i32 x1, i32 y1, i32 x2, i32 y2)
{
	if (!gEnv->p3DEngine->GetITerrain())
		return;

	GetIEditorImpl()->SetModifiedFlag();

	AABB bounds;
	bounds.Reset();
	if (!(x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0))
	{
		// Here we are making sure that we will update the whole sectors where the heightmap was changed.
		i32 nTerrainSectorSize(gEnv->p3DEngine->GetTerrainSectorSize());
		assert(nTerrainSectorSize > 0);

		x1 *= m_heightmap.GetUnitSize();
		y1 *= m_heightmap.GetUnitSize();

		x2 *= m_heightmap.GetUnitSize();
		y2 *= m_heightmap.GetUnitSize();

		x1 /= nTerrainSectorSize;
		y1 /= nTerrainSectorSize;
		x2 /= nTerrainSectorSize;
		y2 /= nTerrainSectorSize;

		// Y and X switched by historical reasons.
		bounds.Add(Vec3((y1 - 1) * nTerrainSectorSize, (x1 - 1) * nTerrainSectorSize, -32000.0f));
		bounds.Add(Vec3((y2 + 1) * nTerrainSectorSize, (x2 + 1) * nTerrainSectorSize, +32000.0f));
	}
}

void CTerrainManager::SelectLayer(i32 layerIndex)
{
	for (CLayer* pLayer : m_layers)
		pLayer->SetSelected(false);

	CLayer* pSelectedLayer = nullptr;
	if (0 <= layerIndex && m_layers.size() > layerIndex)
	{
		pSelectedLayer = m_layers[layerIndex];
		if (pSelectedLayer)
			pSelectedLayer->SetSelected(true);
	}

	signalSelectedLayerChanged(pSelectedLayer);
}

i32 CTerrainManager::GetSelectedLayerIndex()
{
	for (i32 i = 0; i < m_layers.size(); ++i)
	{
		CLayer* pLayer = m_layers[i];
		if (pLayer->IsSelected())
		{
			return i;
		}
	}

	return -1;
}

CLayer* CTerrainManager::GetSelectedLayer()
{
	i32 idx = GetSelectedLayerIndex();
	if (0 <= idx && m_layers.size() > idx)
		return m_layers[idx];

	return nullptr;
}

void CTerrainManager::AddLayer(CLayer* layer)
{
	i32 layerIndex = m_layers.size();
	m_layers.push_back(layer);
	signalLayersChanged();

	SelectLayer(layerIndex);
}

