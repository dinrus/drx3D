// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Util/PakFile.h"
#include "Util/Image.h"
#include "QT/Widgets/QWaitProgress.h"

enum EGameExport
{
	eExp_SurfaceTexture   = BIT(0),
	eExp_ReloadTerrain    = BIT(1),
	eExp_AI_CoverSurfaces = BIT(2),
	eExp_AI_MNM           = BIT(3),
	eExp_Fast             = BIT(4),

	eExp_AI_All           = eExp_AI_MNM | eExp_AI_CoverSurfaces,
};

class CTerrainLightGen;
class CWaitProgress;

struct SGameExporterSettings
{
	bool    bHiQualityCompression;
	bool    bUpdateIndirectLighting;
	UINT    iExportTexWidth;
	i32     nApplySS;
	float   fBrMultiplier;
	EEndian eExportEndian;

	SGameExporterSettings();
	void SetLowQuality();
	void SetHiQuality();
};

struct SANDBOX_API SLevelPakHelper
{
	SLevelPakHelper() : m_bPakOpened(false), m_bPakOpenedDrxPak(true) {}

	string   m_sPath;
	CPakFile m_pakFile;
	bool     m_bPakOpened;
	bool     m_bPakOpenedDrxPak;
};

/*!	CGameExporter implements exporting of data from Editor to Game format.
    It will produce level.pak file in current level folder, with necessary exported files.
 */
class SANDBOX_API CGameExporter
{
public:
	CGameExporter(const SGameExporterSettings& settings = SGameExporterSettings());
	~CGameExporter();

	const SGameExporterSettings& GetSettings() const { return m_settings; }

	SLevelPakHelper&             GetLevelPack()      { return m_levelPak; }

	// In auto exporting mode, highest possible settings will be chosen and no UI dialogs will be shown.
	void                  SetAutoExportMode(bool bAuto) { m_bAutoExportMode = bAuto; }

	bool                  Export(u32 flags = 0, tukk subdirectory = 0);
	bool                  ExportSvogiData();

	static CGameExporter* GetCurrentExporter();

private: // ----------------------------------------------------------------------
	static tukk GetLevelPakFilename()     { return "level.pak"; }
	static tukk GetSvogiDataPakFilename() { return "svogi.pak"; }
	static void        EncryptPakFile(string& rPakFilename);

	bool               OpenLevelPack(SLevelPakHelper& lphelper, bool bDrxPak = false);
	bool               CloseLevelPack(SLevelPakHelper& lphelper, bool bDrxPak = false);

	bool               DoExport(u32 flags = 0, tukk subdirectory = 0);
	void               ExportLevelData(const string& path, bool bExportMission = true);
	void               ExportLevelInfo(const string& path);

	void               ExportBrushes(const string& path);
	bool               ExportSurfaceTexture(CPakFile& levelPakFile, tukk szFilePathNamefloat, float fLeft = 0.0f, float fTop = 0.0f, float fWidth = 1.0f, float fHeight = 1.0f);
	bool               ExportMap(tukk pszGamePath, bool bSurfaceTexture, EEndian eExportEndian);
	void               ExportMergedMeshInstanceSectors(tukk pszGamePath, EEndian eExportEndian, std::vector<struct IStatInstGroup*>* pVegGroupTable);
	void               ExportDynTexSrcLayerInfo(tukk pszGamePath);
	void               ExportOcclusionMesh(tukk pszGamePath);
	void               ExportHeightMap(tukk pszGamePath, EEndian eExportEndian);
	void               ExportAnimations(const string& path);
	void               ExportMapInfo(XmlNodeRef& node);
	string             ExportAI(const string& path, u32 flags);

	void               ExportLevelLensFlares(const string& path);
	void               ExportLevelResourceList(const string& path);
	void               ExportLevelPerLayerResourceList(const string& path);
	void               ExportLevelShaderCache(const string& path);
	void               ExportMaterials(XmlNodeRef& levelDataNode, const string& path);
	void               ExportPrefabs(XmlNodeRef& levelDataNode, const string& path);
	void               ExportGameTokens(XmlNodeRef& levelDataNode, const string& path);

	// create a filelist.xml, with
	void   ExportFileList(const string& path, const string& levelName);

	string ExportAIAreas(const string& path);
	string ExportAICoverSurfaces(const string& path);
	void   ExportAINavigationData(const string& path);

	void   GatherLayerResourceList_r(CObjectLayer* pLayer, CUsedResources& resources);

	void   ExportGameData(const string& path);

	struct SRecursionHelperQuarter
	{
		CImageEx m_Layer[2];
		float    fTerrainMinZ, fTerrainMaxZ;
	};

	struct SRecursionHelper
	{
		// 4 textures in one (0=lefttop, 1=righttop, 2=leftbottom, 3=rightbottom)
		SRecursionHelperQuarter m_Quarter[4];
	};

	struct SProgressHelper
	{
		SProgressHelper(CWaitProgress& ref, u32k dwMax, CTerrainLightGen& rLightGen, CFile& toFile,
		                std::vector<i16>& rIndexBlock, const ULONGLONG FileTileOffset, const ULONGLONG ElementFileSize, tukk szFilename)
			: m_Progress(ref), m_Current(0), m_Max(dwMax), m_rLightGen(rLightGen), m_toFile(toFile),
			m_rIndexBlock(rIndexBlock), m_IndexBlockPos(0), m_FileTileOffset(FileTileOffset), m_ElementFileSize(ElementFileSize),
			m_szFilename(szFilename)
		{
			m_dwLayerExtends[0] = m_dwLayerExtends[1] = 0;
		}

		// Return
		//   true to continue, false to abort lengthy operation.
		bool Increase()
		{
			++m_Current;

			return m_Progress.Step((m_Current * 100) / m_Max);
		}

		u32              m_dwLayerExtends[2];        // layer 0 and 1 width and height

		u32              m_Current;                  //
		u32              m_Max;                      // used for progress indicator and to compute file size = dwUsedTextureIds
		CWaitProgress&      m_Progress;                 //

		CTerrainLightGen&   m_rLightGen;                //
		CFile&              m_toFile;                   //
		ULONGLONG           m_FileTileOffset;           //
		ULONGLONG           m_ElementFileSize;          //

		std::vector<i16>& m_rIndexBlock;              //
		u32              m_IndexBlockPos;            //
		tukk         m_szFilename;               // points to the filenname

		SRecursionHelper    m_TempMem[32];              //
	};

	void DeleteOldFiles(const string& levelDir, bool bSurfaceTexture);
	void ForceDeleteFile(tukk filename);

	// Quality is good (2x2) downsampling but not the best possible (3x3) because we need to limit access
	// within the texture to create texture efficiently
	// the border colors remain the same to avoid discontinuities with LODs
	// Arguments:
	//   rInAndOut - 4 sub images (0=lefttop,1=righttop,2=leftbottom,3=rightbottom), all of the same size
	static void DownSampleWithBordersPreserved(const CImageEx rIn[4], CImageEx& rOut);
	static void DownSampleWithBordersPreservedAO(const CImageEx rIn[4],
	                                             const float rInTerrainMinZ[4], const float rInTerrainMaxZ[4], CImageEx& rOut);

	// Arguments:
	//   dwMinX - [0..dwSectorCount-1]
	//   dwMinY - [0..dwSectorCount-1]
	// Return
	//   true to continue, false to abort lengthy operation.
	bool _ExportSurfaceTextureRecursive(CPakFile& levelPakFile, CDrxMemFile& fileTemp, SProgressHelper& phelper, i32k iParentIndex, SRecursionHelperQuarter& rDestPiece,
	                                    const float fMinX = 0, const float fMinY = 0, const float fWidth = 1.0f, const float fHeight = 1.0f, u32k dwRecursionDepth = 0);

	bool _BuildIndexBlockRecursive(u32k dwTileSize, std::vector<i16>& rIndexBlock, u32& dwUsedTextureIds,
	                               const float fMinX = 0, const float fMinY = 0, const float fWidth = 1.0f, const float fHeight = 1.0f, u32k dwRecursionDepth = 0);

	// Return
	//   true to continue, false to abort lengthy operation.
	bool SeekToSectorAndStore(CPakFile& levelPakFile, CDrxMemFile& fileTemp, SProgressHelper& phelper, i32k iParentIndex, u32k dwLevel,
	                          SRecursionHelperQuarter& rQuarter);

	void BuildSectorAOData(i32 nAreaX, i32 nAreaY, i32 nAreaSize, struct SAOInfo* pData, i32 nDataSize, float fTerrainMinZ, float fTerrainMaxZ);

	void Error(const string& error);

private:
	string                m_levelPath;
	SLevelPakHelper       m_levelPak;
	SLevelPakHelper       m_SvogiDataPak;
	SGameExporterSettings m_settings;

	bool                  m_bAutoExportMode;
	i32                   m_numExportedMaterials;

	static CGameExporter* m_pCurrentExporter;
};

