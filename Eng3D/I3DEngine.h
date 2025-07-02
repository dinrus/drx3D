// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//! The maximum number of unique surface types that can be used per node.
#define MMRM_MAX_SURFACE_TYPES 16

// Do not add any headers here!
#include "DinrusXDecalInfo.h"
#include <drx3D/Eng3D/IStatObj.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/Sys/IProcess.h>
#include <drx3D/Eng3D/IMaterial.h>
#include <drx3D/Eng3D/ISurfaceType.h>
#include <drx3D/Eng3D/IRenderNode.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Memory/IMemory.h>
//Do not add any headers here!

struct ISystem;
struct ICharacterInstance;
struct CVars;
struct pe_params_particle;
struct IMaterial;
struct RenderLMData;
struct AnimTexInfo;
struct ISplineInterpolator;
class CContentCGF;
struct SpawnParams;
class IDrxSizer;
struct IRenderNode;
struct SRenderNodeTempData;
struct IParticleUpr;
class IOpticsUpr;
struct IDeferredPhysicsEventUpr;
struct IBSPTree3D;
struct ITimeOfDay;
struct IRenderView;
class CRenderView;

namespace ChunkFile
{
struct IChunkFileWriter;
}

const float HDRDynamicMultiplier = 2.0f;

enum E3DEngineParameter
{
	E3DPARAM_SUN_COLOR,
	E3DPARAM_SKY_COLOR,

	E3DPARAM_SUN_SPECULAR_MULTIPLIER,

	E3DPARAM_AMBIENT_GROUND_COLOR,
	E3DPARAM_AMBIENT_MIN_HEIGHT,
	E3DPARAM_AMBIENT_MAX_HEIGHT,

	E3DPARAM_FOG_COLOR,
	E3DPARAM_FOG_COLOR2,
	E3DPARAM_FOG_RADIAL_COLOR,

	E3DPARAM_VOLFOG_HEIGHT_DENSITY,
	E3DPARAM_VOLFOG_HEIGHT_DENSITY2,

	E3DPARAM_VOLFOG_GRADIENT_CTRL,

	E3DPARAM_VOLFOG_GLOBAL_DENSITY,
	E3DPARAM_VOLFOG_RAMP,

	E3DPARAM_VOLFOG_SHADOW_RANGE,
	E3DPARAM_VOLFOG_SHADOW_DARKENING,
	E3DPARAM_VOLFOG_SHADOW_ENABLE,

	E3DPARAM_VOLFOG2_CTRL_PARAMS,
	E3DPARAM_VOLFOG2_SCATTERING_PARAMS,
	E3DPARAM_VOLFOG2_RAMP,
	E3DPARAM_VOLFOG2_COLOR,
	E3DPARAM_VOLFOG2_GLOBAL_DENSITY,
	E3DPARAM_VOLFOG2_HEIGHT_DENSITY,
	E3DPARAM_VOLFOG2_HEIGHT_DENSITY2,
	E3DPARAM_VOLFOG2_COLOR1,
	E3DPARAM_VOLFOG2_COLOR2,

	E3DPARAM_SKYLIGHT_RAYLEIGH_INSCATTER,

	E3DPARAM_NIGHSKY_HORIZON_COLOR,
	E3DPARAM_NIGHSKY_ZENITH_COLOR,
	E3DPARAM_NIGHSKY_ZENITH_SHIFT,

	E3DPARAM_NIGHSKY_STAR_INTENSITY,

	E3DPARAM_NIGHSKY_MOON_DIRECTION,
	E3DPARAM_NIGHSKY_MOON_COLOR,
	E3DPARAM_NIGHSKY_MOON_SIZE,
	E3DPARAM_NIGHSKY_MOON_INNERCORONA_COLOR,
	E3DPARAM_NIGHSKY_MOON_INNERCORONA_SCALE,
	E3DPARAM_NIGHSKY_MOON_OUTERCORONA_COLOR,
	E3DPARAM_NIGHSKY_MOON_OUTERCORONA_SCALE,

	E3DPARAM_CLOUDSHADING_MULTIPLIERS,
	E3DPARAM_CLOUDSHADING_SUNCOLOR,
	E3DPARAM_CLOUDSHADING_SKYCOLOR,

	E3DPARAM_VOLCLOUD_ATMOSPHERIC_SCATTERING,
	E3DPARAM_VOLCLOUD_GEN_PARAMS,
	E3DPARAM_VOLCLOUD_SCATTERING_LOW,
	E3DPARAM_VOLCLOUD_SCATTERING_HIGH,
	E3DPARAM_VOLCLOUD_GROUND_COLOR,
	E3DPARAM_VOLCLOUD_SCATTERING_MULTI,
	E3DPARAM_VOLCLOUD_WIND_ATMOSPHERIC,
	E3DPARAM_VOLCLOUD_TURBULENCE,
	E3DPARAM_VOLCLOUD_ENV_PARAMS,
	E3DPARAM_VOLCLOUD_GLOBAL_NOISE_SCALE,
	E3DPARAM_VOLCLOUD_RENDER_PARAMS,
	E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_SCALE,
	E3DPARAM_VOLCLOUD_TURBULENCE_NOISE_PARAMS,
	E3DPARAM_VOLCLOUD_DENSITY_PARAMS,
	E3DPARAM_VOLCLOUD_MISC_PARAM,
	E3DPARAM_VOLCLOUD_TILING_SIZE,
	E3DPARAM_VOLCLOUD_TILING_OFFSET,

	E3DPARAM_CORONA_SIZE,

	E3DPARAM_OCEANFOG_COLOR,
	E3DPARAM_OCEANFOG_DENSITY,

	//! Sky highlight (ex. From Lightning).
	E3DPARAM_SKY_HIGHLIGHT_COLOR,
	E3DPARAM_SKY_HIGHLIGHT_SIZE,
	E3DPARAM_SKY_HIGHLIGHT_POS,

	E3DPARAM_SKY_MOONROTATION,

	E3DPARAM_SKY_SKYBOX_ANGLE,
	E3DPARAM_SKY_SKYBOX_STRETCHING,

	EPARAM_SUN_SHAFTS_VISIBILITY,

	E3DPARAM_SKYBOX_MULTIPLIER,

	E3DPARAM_DAY_NIGHT_INDICATOR,

	//! Tone mapping tweakables.
	E3DPARAM_HDR_FILMCURVE_SHOULDER_SCALE,
	E3DPARAM_HDR_FILMCURVE_LINEAR_SCALE,
	E3DPARAM_HDR_FILMCURVE_TOE_SCALE,
	E3DPARAM_HDR_FILMCURVE_WHITEPOINT,

	E3DPARAM_HDR_EYEADAPTATION_PARAMS,
	E3DPARAM_HDR_EYEADAPTATION_PARAMS_LEGACY,
	E3DPARAM_HDR_BLOOM_AMOUNT,

	E3DPARAM_HDR_COLORGRADING_COLOR_SATURATION,
	E3DPARAM_HDR_COLORGRADING_COLOR_BALANCE,

	E3DPARAM_COLORGRADING_FILTERS_PHOTOFILTER_COLOR,
	E3DPARAM_COLORGRADING_FILTERS_PHOTOFILTER_DENSITY,
	E3DPARAM_COLORGRADING_FILTERS_GRAIN
};

enum EShadowMode
{
	ESM_NORMAL = 0,
	ESM_HIGHQUALITY
};

//! \cond INTERNAL
//! This structure is filled and passed by the caller to the DebugDraw functions of the stat object or entity.
struct SGeometryDebugDrawInfo
{
	Matrix34 tm;        //!< Transformation Matrix.
	ColorB   color;     //!< Optional color of the lines.
	ColorB   lineColor; //!< Optional color of the lines.

	//! Optional flags controlling how to render debug draw information.
	u32 bNoCull      : 1;
	u32 bNoLines     : 1;
	u32 bDrawInFront : 1;   //!< Draw debug draw geometry on top of real geometry.

	SGeometryDebugDrawInfo() : color(255, 0, 255, 255), lineColor(255, 255, 0, 255), bNoLines(0), bNoCull(0) { tm.SetIdentity(); }
};
//! \endcond

struct SFrameLodInfo
{
	u32 nID;
	float  fLodRatio;
	float  fTargetSize;
	uint   nMinLod;
	uint   nMaxLod;

	SFrameLodInfo()
	{
		nID = 0;
		fLodRatio = 0.f;
		fTargetSize = 0.f;
		nMinLod = 0;
		nMaxLod = 6;
	}
};

struct SMeshLodInfo
{
	static i32k s_nMaxLodCount = 5;

	float            fGeometricMean;
	uint             nFaceCount;
	u32           nFrameLodID;

	SMeshLodInfo()
	{
		Clear();
	}

	void Clear()
	{
		fGeometricMean = 0.f;
		nFaceCount = 0;
		nFrameLodID = 0;
	}

	void Merge(const SMeshLodInfo& lodInfo)
	{
		uint nTotalCount = nFaceCount + lodInfo.nFaceCount;
		if (nTotalCount > 0)
		{
			float fGeometricMeanTotal = 0.f;

			if (fGeometricMean > 0.f)
			{
				fGeometricMeanTotal += logf(fGeometricMean) * nFaceCount;
			}
			if (lodInfo.fGeometricMean > 0.f)
			{
				fGeometricMeanTotal += logf(lodInfo.fGeometricMean) * lodInfo.nFaceCount;
			}

			fGeometricMean = expf(fGeometricMeanTotal / (float)nTotalCount);
			nFaceCount = nTotalCount;
		}
	}
};

//! \cond INTERNAL
//! Physics material enumerator, allows for 3dengine to get material id from game code.
struct IPhysMaterialEnumerator
{
	// <interfuscator:shuffle>
	virtual ~IPhysMaterialEnumerator(){}
	virtual i32         EnumPhysMaterial(tukk szPhysMatName) = 0;
	virtual bool        IsCollidable(i32 nMatId) = 0;
	virtual i32         GetMaterialCount() = 0;
	virtual tukk GetMaterialNameByIndex(i32 index) = 0;
	// </interfuscator:shuffle>
};
//! \endcond

//! Physics foreign data flags.
enum EPhysForeignFlags
{
	PFF_HIDABLE             = 0x1,
	PFF_HIDABLE_SECONDARY   = 0x2,
	PFF_EXCLUDE_FROM_STATIC = 0x4,
	PFF_BRUSH               = 0x8,
	PFF_VEGETATION          = 0x10,
	PFF_UNIMPORTANT         = 0x20,
	PFF_OUTDOOR_AREA        = 0x40,
	PFF_MOVING_PLATFORM     = 0x80,
};

//! Ocean data flags.
enum EOceanRenderFlags
{
	OCR_NO_DRAW             = 1 << 0,
	OCR_OCEANVOLUME_VISIBLE = 1 << 1,
};

//! \cond INTERNAL
//! Structure to pass vegetation group properties.
struct IStatInstGroup
{
	enum EPlayerHideable
	{
		ePlayerHideable_None = 0,
		ePlayerHideable_High,
		ePlayerHideable_Mid,
		ePlayerHideable_Low,

		ePlayerHideable_COUNT,
	};

	IStatInstGroup()
	{
		pStatObj = 0;
		szFileName[0] = 0;
		bHideability = 0;
		bHideabilitySecondary = 0;
		bPickable = 0;
		fBending = 0;
		nCastShadowMinSpec = 0;
		bDynamicDistanceShadows = false;
		bGIMode = true;
		bInstancing = true;
		fSpriteDistRatio = 1.f;
		fShadowDistRatio = 1.f;
		fMaxViewDistRatio = 1.f;
		fLodDistRatio = 1.f;
		fBrightness = 1.f;
		pMaterial = 0;
		bUseSprites = true;
		fDensity = 1;
		fElevationMax = 4096;
		fElevationMin = 8;
		fSize = 1;
		fSizeVar = 0;
		fSlopeMax = 255;
		fSlopeMin = 0;
		fStiffness = 0.5f;
		fDamping = 2.5f;
		fVariance = 0.6f;
		fAirResistance = 1.f;
		bRandomRotation = false;
		nRotationRangeToTerrainNormal = 0;
		nMaterialLayers = 0;
		bAllowIndoor = false;
		bUseTerrainColor = false;
		fAlignToTerrainCoefficient = 0.f;
		bAutoMerged = false;
		minConfigSpec = (ESystemConfigSpec)0;
		nTexturesAreStreamedIn = 0;
		nPlayerHideable = ePlayerHideable_None;
		nID = -1;
	}

	_smart_ptr<IStatObj> pStatObj;
	char                 szFileName[256];
	bool                 bHideability;
	bool                 bHideabilitySecondary;
	bool                 bPickable;
	float                fBending;
	u8                nCastShadowMinSpec;
	bool                 bDynamicDistanceShadows;
	bool                 bGIMode;
	bool                 bInstancing;
	float                fSpriteDistRatio;
	float                fLodDistRatio;
	float                fShadowDistRatio;
	float                fMaxViewDistRatio;
	float                fBrightness;
	bool                 bUseSprites;
	bool                 bRandomRotation;
	i32                nRotationRangeToTerrainNormal;
	float                fAlignToTerrainCoefficient;
	bool                 bUseTerrainColor;
	bool                 bAllowIndoor;
	bool                 bAutoMerged;
	float                fDensity;
	float                fElevationMax;
	float                fElevationMin;
	float                fSize;
	float                fSizeVar;
	float                fSlopeMax;
	float                fSlopeMin;
	float                fStiffness;
	float                fDamping;
	float                fVariance;
	float                fAirResistance;
	float                fVegRadius;
	float                fVegRadiusVert;
	float                fVegRadiusHor;
	i32                  nPlayerHideable;
	i32                  nID;

	//! Minimal configuration spec for this vegetation group.
	ESystemConfigSpec minConfigSpec;

	//! Override material for this instance group.
	_smart_ptr<IMaterial> pMaterial;

	//! Material layers bitmask -> which layers are active.
	u8 nMaterialLayers;

	//! Textures Are Streamed In.
	u8 nTexturesAreStreamedIn;

	//! Flags similar to entity render flags.
	i32 m_dwRndFlags;
};
//! \endcond

//! Interface to water volumes.
//! Water volumes should usually be created by I3DEngine::CreateWaterVolume.
struct IWaterVolume
{
	// <interfuscator:shuffle>
	virtual ~IWaterVolume(){}
	virtual void        UpdatePoints(const Vec3* pPoints, i32 nCount, float fHeight) = 0;
	virtual void        SetFlowSpeed(float fSpeed) = 0;
	virtual void        SetAffectToVolFog(bool bAffectToVolFog) = 0;
	virtual void        SetTriSizeLimits(float fTriMinSize, float fTriMaxSize) = 0;
	//	virtual void SetMaterial(tukk  szShaderName) = 0;
	virtual void        SetMaterial(IMaterial* pMaterial) = 0;
	virtual IMaterial*  GetMaterial() = 0;
	virtual tukk GetName() const = 0;
	virtual void        SetName(tukk szName) = 0;

	//! Sets a new water level.
	//! Used to change the water level. Will assign a new Z value to all
	//! vertices of the water geometry.
	//! \param vNewOffset - Position of the new water level
	virtual void SetPositionOffset(const Vec3& vNewOffset) = 0;
	// </interfuscator:shuffle>
};

struct IClipVolume
{
	enum { MaxBlendPlaneCount = 2 };
	enum EClipVolumeFlags
	{
		eClipVolumeConnectedToOutdoor = BIT(0),
		eClipVolumeIgnoreGI           = BIT(1),
		eClipVolumeAffectedBySun      = BIT(2),
		eClipVolumeBlend              = BIT(3),
		eClipVolumeIsVisArea          = BIT(4),
		eClipVolumeIgnoreOutdoorAO    = BIT(5),
	};

	virtual ~IClipVolume() {};
	virtual void         GetClipVolumeMesh(_smart_ptr<IRenderMesh>& renderMesh, Matrix34& worldTM) const = 0;
	virtual const AABB&  GetClipVolumeBBox() const = 0;
	virtual bool         IsPointInsideClipVolume(const Vec3& point) const = 0;

	virtual u8 GetStencilRef() const = 0;
	virtual uint  GetClipVolumeFlags() const = 0;
};

//! Provides information about the different VisArea volumes.
struct IVisArea : public IClipVolume
{
	// <interfuscator:shuffle>
	virtual ~IVisArea(){}
	//! Gets the last rendered frame id.
	//! \return An i32 which contains the frame id.
	virtual i32 GetVisFrameId() = 0;

	//! Gets all the areas which are connected to the current one.
	//! Gets a list of all the VisAreas which are connected to the current one.
	//! If the return is equal to nMaxConnNum, it's possible that not all
	//! connected VisAreas were returned due to the restriction imposed by the argument.
	//! \param pAreas               - Pointer to an array of IVisArea*
	//! \param nMaxConnNum          - The maximum of IVisArea to write in pAreas
	//! \param bSkipDisabledPortals - Ignore portals which are disabled
	//! \return An integer which hold the amount of VisArea found to be connected.
	virtual i32 GetVisAreaConnections(IVisArea** pAreas, i32 nMaxConnNum, bool bSkipDisabledPortals = false) = 0;

	//! Determines if it's connected to an outdoor area.
	//! \return True if the VisArea is connected to an outdoor area.
	virtual bool IsConnectedToOutdoor() const = 0;

	//! Determines if the visarea ignores Global Illumination inside.
	//! \return True if the VisArea ignores Global Illumination inside.
	virtual bool IsIgnoringGI() const = 0;

	//! Determines if the visarea ignores Outdoor Ambient occlusion inside.
	//! \return True if the VisArea ignores Outdoor Ambient Occlusion inside.
	virtual bool IsIgnoringOutdoorAO() const = 0;

	//! Gets the name.
	//! \note The name is always returned in lower case.
	//! \return A null terminated char array containing the name of the VisArea.
	virtual tukk GetName() = 0;

	//! Determines if this VisArea is a portal.
	//! \return True if the VisArea is a portal, or false in the opposite case.
	virtual bool IsPortal() const = 0;

	//! Searches for a specified VisArea.
	//! Searches for a specified VisArea to see if it's connected to the current VisArea.
	//! \param pAnotherArea         A specified VisArea to find
	//! \param nMaxRecursion        The maximum number of recursion to do while searching
	//! \param bSkipDisabledPortals Will avoid searching disabled VisAreas
	//! \param pVisitedAreas		If not NULL - will get list of all visited areas
	//! \return True if the VisArea was found.
	virtual bool FindVisArea(IVisArea* pAnotherArea, i32 nMaxRecursion, bool bSkipDisabledPortals) = 0;

	//! Searches for the surrounding VisAreas.
	//! Searches for the surrounding VisAreas which connected to the current VisArea.
	//! \param nMaxRecursion        The maximum number of recursion to do while searching
	//! \param bSkipDisabledPortals Will avoid searching disabled VisAreas
	//! \param pVisitedAreas		if not NULL - will get list of all visited areas
	virtual void FindSurroundingVisArea(i32 nMaxRecursion, bool bSkipDisabledPortals, PodArray<IVisArea*>* pVisitedAreas = NULL, i32 nMaxVisitedAreas = 0, i32 nDeepness = 0) = 0;

	//! Determines if it's affected by outdoor lighting.
	//! \return true if the VisArea if it's affected by outdoor lighting, else false will be returned.
	virtual bool IsAffectedByOutLights() const = 0;

	//! Determines if the spere can be affect the VisArea.
	//! \return Returns true if the VisArea can be affected by the sphere, else false will be returned.
	virtual bool IsSphereInsideVisArea(const Vec3& vPos, const f32 fRadius) = 0;

	//! Clips geometry inside or outside a vis area.
	//! \return true if geom was clipped.
	virtual bool ClipToVisArea(bool bInside, Sphere& sphere, Vec3 const& vNormal) = 0;

	//! Gives back the axis aligned bounding box of VisArea.
	//! \return the pointer of an AABB.
	virtual const AABB* GetAABBox() const = 0;

	//! Gives back the axis aligned bounding box of all static objects in the VisArea.
	//! This AABB can be larger than the ViaArea AABB as some objects might not be completely inside the VisArea.
	//! \return Returns the pointer to the AABB.
	virtual const AABB* GetStaticObjectAABBox() const = 0;

	//! Determines if the point can be affect the VisArea.
	//! \return Returns true if the VisArea can be affected by the point, else false will be returned.
	virtual bool IsPointInsideVisArea(const Vec3& vPos) const = 0;

	//! \return vis area final ambient color (ambient color depends on factors, like if connected to outdoor, is affected by skycolor - etc)
	virtual const Vec3 GetFinalAmbientColor() = 0;

	virtual void       GetShapePoints(const Vec3*& pPoints, size_t& nPoints) = 0;
	virtual float      GetHeight() = 0;
	// </interfuscator:shuffle>
};

//! Water level unknown.
#define WATER_LEVEL_UNKNOWN  -1000000.f
#define BOTTOM_LEVEL_UNKNOWN -1000000.f

//! Float m_SortId		: offset by +WATER_LEVEL_SORTID_OFFSET if the camera object line is crossing the water surface,
//!                     : otherwise offset by -WATER_LEVEL_SORTID_OFFSET.
#define WATER_LEVEL_SORTID_OFFSET 10000000

//! Indirect lighting quadtree definition.
namespace NQT
{
// Forward declaration
template<class TLeafContent, u32 TMaxCellElems, class TPosType, class TIndexType, bool TUseRadius>
class CQuadTree;
}

#define FILEVERSION_TERRAIN_SHLIGHTING_FILE 5

enum EVoxelBrushShape
{
	evbsSphere = 1,
	evbsBox,
};

enum EVoxelEditTarget
{
	evetVoxelObjects = 1,
};

enum EVoxelEditOperation
{
	eveoNone = 0,
	eveoPaintHeightPos,
	eveoPaintHeightNeg,
	eveoCreate,
	eveoSubstract,
	eveoMaterial,
	eveoBaseColor,
	eveoBlurPos,
	eveoBlurNeg,
	eveoCopyTerrainPos,
	eveoCopyTerrainNeg,
	eveoPickHeight,
	eveoIntegrateMeshPos,
	eveoIntegrateMeshNeg,
	eveoForceDepth,
	eveoLimitLod,
	eveoLast,
};

#define COMPILED_HEIGHT_MAP_FILE_NAME      "terrain\\terrain.dat"
#define COMPILED_VISAREA_MAP_FILE_NAME     "terrain\\indoor.dat"
#define COMPILED_TERRAIN_TEXTURE_FILE_NAME "terrain\\cover.ctc"
#define COMPILED_SVO_FOLDER_NAME           "terrain\\svo\\"
#define COMPILED_MERGED_MESHES_BASE_NAME   "terrain\\merged_meshes_sectors\\"
#define COMPILED_MERGED_MESHES_LIST        "mmrm_used_meshes.lst"
#define LEVEL_INFO_FILE_NAME               "levelinfo.xml"

//////////////////////////////////////////////////////////////////////////

#pragma pack(push,4)

struct STerrainInfo
{
	i32   heightMapSize_InUnits;
	float unitSize_InMeters;
	i32   sectorSize_InMeters;

	i32   sectorsTableSize_InSectors;
	float heightmapZRatio;
	float oceanWaterLevel;

	AUTO_STRUCT_INFO;
};

#define TERRAIN_CHUNK_VERSION                                29
#define VISAREAMANAGER_CHUNK_VERSION                         6

#define SERIALIZATION_FLAG_BIG_ENDIAN                        1
#define SERIALIZATION_FLAG_SECTOR_PALETTES                   2
#define SERIALIZATION_FLAG_INSTANCES_PRESORTED_FOR_STREAMING 4

#define TCH_FLAG2_AREA_ACTIVATION_IN_USE                     1

struct STerrainChunkHeader
{
	int8         nVersion;
	int8         nDummy;
	int8         nFlags;
	int8         nFlags2;
	i32        nChunkSize;
	STerrainInfo TerrainInfo;

	AUTO_STRUCT_INFO;
};

struct SVisAreaManChunkHeader
{
	int8 nVersion;
	int8 nDummy;
	int8 nFlags;
	int8 nFlags2;
	i32  nChunkSize;
	i32  nVisAreasNum;
	i32  nPortalsNum;
	i32  nOcclAreasNum;

	AUTO_STRUCT_INFO;
};

struct SOcTreeNodeChunk
{
	i16 nChunkVersion;
	i16 ucChildsMask;
	AABB  nodeBox;
	i32 nObjectsBlockSize;

	AUTO_STRUCT_INFO;
};

struct IEditorHeightmap
{
	// <interfuscator:shuffle>
	virtual ~IEditorHeightmap(){}
	virtual u32 GetDominatingLayerIdAtPosition(i32k x, i32k y) const = 0;
	virtual u32 GetDominatingSurfaceTypeIdAtPosition(i32k x, i32k y) const = 0;
	virtual bool   GetHoleAtPosition(i32k x, i32k y) const = 0;
	virtual ColorB GetColorAtPosition(const float x, const float y, ColorB* colors = nullptr, i32k colorsNum = 0, const float xStep = 0) = 0;
	virtual float  GetElevationAtPosition(const float x, const float y) = 0;
	virtual float  GetRGBMultiplier() = 0;
	// </interfuscator:shuffle>
};

#define TERRAIN_DEFORMATION_MAX_DEPTH 3.f

struct SHotUpdateInfo
{
	SHotUpdateInfo()
	{
		nHeigtmap = 1;
		nObjTypeMask = ~0;
		pVisibleLayerMask = NULL;
		pLayerIdTranslation = NULL;
		areaBox.Reset();
	}

	u32        nHeigtmap;
	u32        nObjTypeMask;
	u8k*  pVisibleLayerMask;
	u16k* pLayerIdTranslation;
	AABB          areaBox;

	AUTO_STRUCT_INFO;
};

//! \cond INTERNAL
//! This structure is used by the editor for storing and editing of terrain surface types
struct SSurfaceTypeItem
{
	//! Maximum number surface types stored in one heightmap uint item
	enum { kMaxSurfaceTypesNum = 3 };

	//! Default constructor
	SSurfaceTypeItem()
	{
	}

	//! Construct from single surface type
	SSurfaceTypeItem(u32 surfType)
	{
		*this = surfType;
	}

	//! Return surface type with highest weight
	u32 GetDominatingSurfaceType() const
	{
		return ty[0];
	}

	//! Mark as hole
	void SetHole(bool enabled)
	{
		hole = enabled ? 255 : 0;
	}

	//! Return true if terrain has hole here
	bool GetHole() const
	{
		return hole == 255;
	}

	//! Check if specified surface type is used
	bool HasType(u32 type) const
	{
		return (ty[0] == type) || (ty[1] == type) || (ty[2] == type);
	}

	//! Assign single specified surface type (clean previous state)
	const SSurfaceTypeItem& operator=(u32 nSurfType)
	{
		ZeroStruct(*this);
		we[0] = 255;
		ty[0] = nSurfType;
		return *this;
	}

	//! Surface type id's
	u8 ty[3] = { 0 };

	//! Is it hole
	u8 hole = 0;

	//! Surface type weights
	u8 we[3] = { 0 };

	//! Not used for now
	u8 dummy = 0;
};
//! \endcond

//! Interface to terrain engine
struct ITerrain
{
	struct SExportInfo
	{
		SExportInfo()
		{
			bHeigtmap = bObjects = true;
			areaBox.Reset();
		}
		bool bHeigtmap;
		bool bObjects;
		AABB areaBox;
	};

	// <interfuscator:shuffle>
	virtual ~ITerrain(){}
	//! Loads data into terrain engine from memory block.
	virtual bool SetCompiledData(byte* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, bool bHotUpdate = false, SHotUpdateInfo* pExportInfo = NULL) = 0;

	//! Saves data from terrain engine into memory block.
	virtual bool GetCompiledData(byte* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, std::vector<struct IStatInstGroup*>** ppStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo = NULL) = 0;

	//! \return terrain data memory block size.
	virtual i32 GetCompiledDataSize(SHotUpdateInfo* pExportInfo = NULL) = 0;

	//! Create and place a new vegetation object on the terrain.
	virtual IRenderNode* AddVegetationInstance(i32 nStaticGroupID, const Vec3& vPos, const float fScale, u8 ucBright, u8 angle, u8 angleX = 0, u8 angleY = 0) = 0;

	//! Set ocean level.
	virtual void SetOceanWaterLevel(float oceanWaterLevel) = 0;

	//! Call this before any calls to CloneRegion to mark all the render nodes in the
	//! source region(s) with the flag ERF_CLONE_SOURCE.  This ensures that the clone
	//! call will only get source nodes, and not cloned ones from multiple calls to
	//! CloneRegion.  The offset is an optional value for offsetting the clone sources,
	//! to ensure they won't be overlapping the clones (or pass zero for no offset).
	virtual void MarkAndOffsetCloneRegion(const AABB& region, const Vec3& offset) = 0;

	//! Clones all objects in a region of the terrain, offsetting and rotating them based on the values passed in.
	//! \param offset - Offset amount, relative to the center of the region passed in.
	//! \param zRotation - Rotation around the z axis, in radians.
	//! \param pIncludeLayers - Optional list of layer ids to include, zero include layers means include objects from any layer
	virtual void CloneRegion(const AABB& region, const Vec3& offset, float zRotation, u16k* pIncludeLayers = NULL, i32 numIncludeLayers = 0) = 0;

	//! Remove all objects that were marked by MarkAndOffsetCloneRegion.
	virtual void ClearCloneSources() = 0;

	//! \return whole terrain lightmap texture id.
	virtual i32 GetTerrainLightmapTexId(Vec4& vTexGenInfo) = 0;

	//! Return terrain texture atlas texture id's.
	virtual void GetAtlasTexId(i32& nTex0, i32& nTex1, i32& nTex2) = 0;

	//! \return object and material table for Exporting.
	virtual void GetStatObjAndMatTables(DynArray<IStatObj*>* pStatObjTable, DynArray<IMaterial*>* pMatTable, DynArray<IStatInstGroup*>* pStatInstGroupTable, u32 nObjTypeMask) = 0;

	//! Updates part of height map.
	//! x1, y1, nSizeX, nSizeY are in terrain units
	//! pTerrainBlock points to a square 2D array with dimensions GetTerrainSize()
	//! by default update only elevation.
	virtual void SetTerrainElevation(i32 x1, i32 y1, i32 nSizeX, i32 nSizeY, float* pTerrainBlock, SSurfaceTypeItem* pSurfaceData, i32 nSurfOrgX, i32 nSurfOrgY, i32 nSurfSizeX, i32 nSurfSizeY, u32* pResolMap, i32 nResolMapSizeX, i32 nResolMapSizeY) = 0;

	//! Checks if it is possible to paint on the terrain with a given surface type ID.
	//! \note Should be called by the editor to avoid overflowing the sector surface type palettes.
	virtual bool CanPaintSurfaceType(i32 x, i32 y, i32 r, u16 usGlobalSurfaceType) = 0;

	//! \return current amount of terrain textures requests for streaming, if more than 0 = there is streaming in progress.
	virtual i32 GetNotReadyTextureNodesCount() = 0;

	//! Retrieves the resource (mostly texture system memory) memory usage for a given region of the terrain.
	//! \param pSizer Pointer to an instance of the DrxSizer object. The purpose of this object is making sure each element is accounted only once.
	//! \param crstAABB -  Is a reference to the bounding box in which region we want to analyze the resources.
	virtual void GetResourceMemoryUsage(IDrxSizer* pSizer, const AABB& crstAABB) = 0;

	//! \return number of used detail texture materials. Fills materials array if materials!=NULL.
	virtual i32 GetDetailTextureMaterials(IMaterial* materials[]) = 0;

	//! Changes the ocean material
	virtual void ChangeOceanMaterial(IMaterial* pMat) = 0;

	//! Request heightmap mesh update in specified area
	//! if pBox == 0 update entire heightmap
	virtual void ResetTerrainVertBuffers(const AABB* pBox) = 0;

	//! Inform terrain engine about terrain painting/sculpting action finish
	virtual void OnTerrainPaintActionComplete() = 0;
};

//! \cond INTERNAL
//! Callbacks interface for higher level segments management.
//! Warning: deprecated Segmented World implementation is not supported by drx3D anymore
struct ISegmentsUpr
{
	enum ESegmentLoadFlags
	{
		slfTerrain    = BIT(1),
		slfVisArea    = BIT(2),
		slfEntity     = BIT(3),
		slfNavigation = BIT(4),

		slfAll        = slfTerrain | slfVisArea | slfEntity | slfNavigation,
	};
	virtual ~ISegmentsUpr() {}
	virtual void WorldVecToGlobalSegVec(const Vec3& inPos, Vec3& outPos, Vec2& outAbsCoords)         {}
	virtual void GlobalSegVecToLocalSegVec(const Vec3& inPos, const Vec2& inAbsCoords, Vec3& outPos) {}
	virtual Vec3 WorldVecToLocalSegVec(const Vec3& inPos)                                            { return Vec3(0, 0, 0); }
	virtual Vec3 LocalToAbsolutePosition(Vec3 const& vPos, f32 fDir = 1.f) const                     { return Vec3(0, 0, 0); }
	virtual void GetTerrainSizeInMeters(i32& x, i32& y)                                              {}
	virtual i32  GetSegmentSizeInMeters()                                                            { return 0; }
	virtual bool CreateSegments(ITerrain* pTerrain)                                                  { return false; }
	virtual bool DeleteSegments(ITerrain* pTerrain)                                                  { return false; }
	virtual bool FindSegment(ITerrain* pTerrain, const Vec3& pt, i32& nSID)                          { return false; }
	virtual bool FindSegmentCoordByID(i32 nSID, i32& x, i32& y)                                      { return false; }
	virtual void ForceLoadSegments(u32 flags)                                               {}
	virtual bool PushEntityToSegment(u32 id, bool bLocal = true)                            { return false; }
	// </interfuscator:shuffle>
};
//! \endcond

struct IVisAreaCallback
{
	// <interfuscator:shuffle>
	virtual ~IVisAreaCallback(){}
	virtual void OnVisAreaDeleted(IVisArea* pVisArea) = 0;
	// </interfuscator:shuffle>
};

struct IVisAreaTestCallback
{
	virtual bool TestVisArea(IVisArea* pVisArea) const = 0;
};

struct IVisAreaUpr
{
	// <interfuscator:shuffle>
	virtual ~IVisAreaUpr(){}
	//! Loads data into VisAreaUpr engine from memory block.
	virtual bool SetCompiledData(u8* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, bool bHotUpdate, SHotUpdateInfo* pExportInfo) = 0;

	//! Saves data from VisAreaUpr engine into memory block.
	virtual bool GetCompiledData(u8* pData, i32 nDataSize, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<IMaterial*>** ppMatTable, std::vector<struct IStatInstGroup*>** ppStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo = NULL) = 0;

	//! \return VisAreaUpr data memory block size.
	virtual i32 GetCompiledDataSize(SHotUpdateInfo* pExportInfo = NULL) = 0;

	//! \return The accumulated number of visareas and portals.
	virtual i32 GetNumberOfVisArea() const = 0;

	//! \return The visarea interface based on the id (0..GetNumberOfVisArea()) it can be a visarea or a portal.
	virtual IVisArea* GetVisAreaById(i32 nID) const = 0;

	virtual void      AddListener(IVisAreaCallback* pListener) = 0;
	virtual void      RemoveListener(IVisAreaCallback* pListener) = 0;

	virtual void      UpdateConnections() = 0;

	//! Clones all vis areas in a region of the level, offsetting and rotating them based
	//! on the values passed in.
	//! \param offset - Offset amount, relative to the center of the region passed in.
	//! \param zRotation - Rotation around the z axis, in radians.
	virtual void CloneRegion(const AABB& region, const Vec3& offset, float zRotation) = 0;

	//! Removes all vis areas in a region of the level.
	virtual void ClearRegion(const AABB& region) = 0;
	// </interfuscator:shuffle>
};

//! \cond INTERNAL
//! Manages simple pre-merged mesh instances into pre-baked sectors.
struct IMergedMeshesUpr
{
	enum CLUSTER_FLAGS
	{
		//! Uses the samples themselves to calculate the bounds of the areacluster.
		//! If not set, it will use the bounds of the patches to calculate the cluster.
		CLUSTER_BOUNDARY_FROM_SAMPLES = 1 << 0,

		//! Create the cluster-area boundary by computing the convex hull of either.
		//! The samples or the bounds of the patches via graham scan.
		CLUSTER_CONVEXHULL_GRAHAMSCAN = 1 << 1,

		//! Create the cluster-area boundary by computing the convex hull of either.
		//! The samples or the bounds of the patches via giftwrapping (also knows as.
		//! Jarvis march).
		//! Note: this might not be as stable as the graham scan method above,.
		//! Especially in terms of collinear points or otherwise degenerate data.
		CLUSTER_CONVEXHULL_GIFTWRAP = 1 << 2,
	};

	struct SInstanceSector
	{
		DynArray<u8> data;    //!< Memory stream of internally compiled data.
		string          id;      //!< Unique identifier string identifing this sector.
	};

	struct SMeshAreaCluster
	{
		//! The extents (in worldspace) of the cluster.
		AABB extents;

		//! The vertices of the boundary points (in the xy-plane, worldspace) of this.
		DynArray<Vec2> boundary_points;
	};

	// <interfuscator:shuffle>
	virtual ~IMergedMeshesUpr() {}

	///////////////////////////////////////////////////////////////////////////////////////
	//! \note The caller is responsible for freeing the allocated memory.
	//! \return false on error.
	virtual bool CompileSectors(std::vector<struct IStatInstGroup*>* pVegGroupTable) = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! \return the compiled instance sector at specified index.
	virtual const IMergedMeshesUpr::SInstanceSector& GetInstanceSector(size_t index) const = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! \return the compiled instance sectors count.
	virtual size_t GetInstanceSectorCount() const = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! Clears the compiled sectors buffer
	virtual void ClearInstanceSectors() = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! \note The caller is responsible for freeing the allocated memory (both the cluster array and the contents).
	//! \return A list of cluster hulls.
	virtual bool CompileAreas(DynArray<SMeshAreaCluster>& clusters, i32 flags) = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! Query the sample density grid. Returns the number of surface types filled into the surface types list.
	typedef float         TFixedDensityArray[MMRM_MAX_SURFACE_TYPES];        // typedef to simplify function for SWIG parser
	typedef ISurfaceType* TFixedSurfacePtrTypeArray[MMRM_MAX_SURFACE_TYPES]; // typedef to simplify function for SWIG parser
	virtual size_t QueryDensity(const Vec3& pos, TFixedSurfacePtrTypeArray& surfaceTypes, TFixedDensityArray& density) = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! Fill in the density values.
	virtual void CalculateDensity() = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! \return The list of merged mesh geometry currently active. Returns false on error.
	virtual bool GetUsedMeshes(DynArray<string>& pMeshNames) = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//! \return The current memory footprint in vram (accumulated vertex and indexbuffer size in bytes).
	virtual size_t CurrentSizeInVram() const = 0;

	//! \return The current memory footprint in main memory (the accumulated footprint of all merged instances).
	virtual size_t CurrentSizeInMainMem() const = 0;

	//! \return The memory footprint of the prebaked geometry in bytes.
	virtual size_t GeomSizeInMainMem() const = 0;

	//! \return The size of the instance map in bytes.
	virtual size_t InstanceSize() const = 0;

	//! \return The size of animated instances if they have spines.
	virtual size_t SpineSize() const = 0;

	//! The instance count.
	virtual size_t InstanceCount() const = 0;

	//! The number of visible instances last frame.
	virtual size_t VisibleInstances() const = 0;
	// </interfuscator:shuffle>
};
//! \endcond

struct IFoliage
{
	// <interfuscator:shuffle>
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual ~IFoliage(){}
	enum EFoliageFlags { FLAG_FROZEN = 1 };
	virtual i32              Serialize(TSerialize ser) = 0;
	virtual void             SetFlags(i32 flags) = 0;
	virtual i32              GetFlags() = 0;
	virtual IRenderNode*     GetIRenderNode() = 0;
	virtual i32              GetBranchCount() = 0;
	virtual IPhysicalEntity* GetBranchPhysics(i32 iBranch) = 0;
	virtual SSkinningData*   GetSkinningData(const Matrix34& RenderMat34, const SRenderingPassInfo& passInfo) = 0;
	// </interfuscator:shuffle>
};

struct SSkyLightRenderParams
{
	static i32k skyDomeTextureWidth = 64;
	static i32k skyDomeTextureHeight = 32;
	static i32k skyDomeTextureSize = 64 * 32;

	static i32k skyDomeTextureWidthBy8 = 8;
	static i32k skyDomeTextureWidthBy4Log = 4;   //!< = log2(64/4).
	static i32k skyDomeTextureHeightBy2Log = 4;  //!< = log2(32/2).

	SSkyLightRenderParams()
		: m_pSkyDomeMesh(0)
		, m_pSkyDomeTextureDataMie(0)
		, m_pSkyDomeTextureDataRayleigh(0)
		, m_skyDomeTexturePitch(0)
		, m_skyDomeTextureTimeStamp(-1)
		, m_partialMieInScatteringConst(0.0f, 0.0f, 0.0f, 0.0f)
		, m_partialRayleighInScatteringConst(0.0f, 0.0f, 0.0f, 0.0f)
		, m_sunDirection(0.0f, 0.0f, 0.0f, 0.0f)
		, m_phaseFunctionConsts(0.0f, 0.0f, 0.0f, 0.0f)
		, m_hazeColor(0.0f, 0.0f, 0.0f, 0.0f)
		, m_hazeColorMieNoPremul(0.0f, 0.0f, 0.0f, 0.0f)
		, m_hazeColorRayleighNoPremul(0.0f, 0.0f, 0.0f, 0.0f)
		, m_skyColorTop(0.0f, 0.0f, 0.0f)
		, m_skyColorNorth(0.0f, 0.0f, 0.0f)
		, m_skyColorEast(0.0f, 0.0f, 0.0f)
		, m_skyColorSouth(0.0f, 0.0f, 0.0f)
		, m_skyColorWest(0.0f, 0.0f, 0.0f)
	{
	}

	//! Sky dome mesh.
	_smart_ptr<IRenderMesh> m_pSkyDomeMesh;

	// temporarily add padding bytes to prevent fetching Vec4 constants below from wrong offset
	u32 dummy0;
	u32 dummy1;

	// Sky dome texture data
	ukk m_pSkyDomeTextureDataMie;
	ukk m_pSkyDomeTextureDataRayleigh;
	size_t      m_skyDomeTexturePitch;
	i32         m_skyDomeTextureTimeStamp;

	i32         pad; //!< Enable 16 byte alignment for Vec4s.

	// Sky dome shader constants
	Vec4 m_partialMieInScatteringConst;
	Vec4 m_partialRayleighInScatteringConst;
	Vec4 m_sunDirection;
	Vec4 m_phaseFunctionConsts;
	Vec4 m_hazeColor;
	Vec4 m_hazeColorMieNoPremul;
	Vec4 m_hazeColorRayleighNoPremul;

	// Sky hemisphere colors
	Vec3 m_skyColorTop;
	Vec3 m_skyColorNorth;
	Vec3 m_skyColorEast;
	Vec3 m_skyColorSouth;
	Vec3 m_skyColorWest;
};

struct sRAEColdData
{
	Vec4 m_RAEPortalInfos[96];                                            //! It stores all data needed to solve the problem between the portals & indirect lighting.
	//	byte												m_OcclLights[MAX_LIGHTS_NUM];
};

struct SVisAreaInfo
{
	float fHeight;
	Vec3  vAmbientColor;
	bool  bAffectedByOutLights;
	bool  bIgnoreSkyColor;
	bool  bSkyOnly;
	float fViewDistRatio;
	bool  bDoubleSide;
	bool  bUseDeepness;
	bool  bUseInIndoors;
	bool  bOceanIsVisible;
	bool  bIgnoreGI;
	bool  bIgnoreOutdoorAO;
	float fPortalBlending;
};

struct SDebugFPSInfo
{
	SDebugFPSInfo() : fAverageFPS(0.0f), fMaxFPS(0.0f), fMinFPS(0.0f)
	{
	}
	float fAverageFPS;
	float fMinFPS;
	float fMaxFPS;
};

//! Common scene rain parameters shared across engine and editor.
struct DRX_ALIGN(16) SRainParams
{
	SRainParams()
		: fAmount(0.f), fCurrentAmount(0.f), fRadius(0.f), nUpdateFrameID(-1), bIgnoreVisareas(false), bDisableOcclusion(false)
		  , matOccTrans(IDENTITY), matOccTransRender(IDENTITY), qRainRotation(IDENTITY), areaAABB(AABB::RESET)
		  , bApplySkyColor(false), fSkyColorWeight(0.5f)
	{
	}

	Matrix44 matOccTrans;         //!< Transformation matrix for rendering into a new occ map.
	Matrix44 matOccTransRender;   //!< Transformation matrix for rendering occluded rain using current occ map.
	Quat qRainRotation;           //!< Quaternion for the scene's rain entity rotation.
	AABB areaAABB;

	Vec3 vWorldPos;
	Vec3 vColor;

	float fAmount;
	float fCurrentAmount;
	float fRadius;

	float fFakeGlossiness;
	float fFakeReflectionAmount;
	float fDiffuseDarkening;

	float fRainDropsAmount;
	float fRainDropsSpeed;
	float fRainDropsLighting;

	float fMistAmount;
	float fMistHeight;

	float fPuddlesAmount;
	float fPuddlesMaskAmount;
	float fPuddlesRippleAmount;
	float fSplashesAmount;

	i32 nUpdateFrameID;
	bool bApplyOcclusion;
	bool bIgnoreVisareas;
	bool bDisableOcclusion;

	bool bApplySkyColor;
	float fSkyColorWeight;
};

struct SSnowParams
{
	SSnowParams()
		: m_vWorldPos(0, 0, 0), m_fRadius(0.0), m_fSnowAmount(0.0), m_fFrostAmount(0.0), m_fSurfaceFreezing(0.0),
		m_nSnowFlakeCount(0), m_fSnowFlakeSize(0.0), m_fSnowFallBrightness(0.0), m_fSnowFallGravityScale(0.0),
		m_fSnowFallWindScale(0.0), m_fSnowFallTurbulence(0.0), m_fSnowFallTurbulenceFreq(0.0)
	{
	}

	Vec3  m_vWorldPos;
	float m_fRadius;

	// Surface params.
	float m_fSnowAmount;
	float m_fFrostAmount;
	float m_fSurfaceFreezing;

	// Snowfall params.
	i32   m_nSnowFlakeCount;
	float m_fSnowFlakeSize;
	float m_fSnowFallBrightness;
	float m_fSnowFallGravityScale;
	float m_fSnowFallWindScale;
	float m_fSnowFallTurbulence;
	float m_fSnowFallTurbulenceFreq;
};

struct IScreenshotCallback
{
	// <interfuscator:shuffle>
	virtual ~IScreenshotCallback(){}
	virtual void SendParameters(uk data, u32 width, u32 height, f32 minx, f32 miny, f32 maxx, f32 maxy) = 0;
	// </interfuscator:shuffle>
};

class IStreamedObjectListener
{
public:
	// <interfuscator:shuffle>
	virtual void OnCreatedStreamedObject(tukk filename, uk pHandle) = 0;
	virtual void OnRequestedStreamedObject(uk pHandle) = 0;
	virtual void OnReceivedStreamedObject(uk pHandle) = 0;
	virtual void OnUnloadedStreamedObject(uk pHandle) = 0;
	virtual void OnBegunUsingStreamedObjects(uk * pHandles, size_t numHandles) = 0;
	virtual void OnEndedUsingStreamedObjects(uk * pHandles, size_t numHandles) = 0;
	virtual void OnDestroyedStreamedObject(uk pHandle) = 0;
	// </interfuscator:shuffle>
protected:
	virtual ~IStreamedObjectListener() {}
};

struct IRenderNodeStatusListener
{
	virtual void OnEntityDeleted(IRenderNode* pRenderNode) = 0;
protected:
	virtual ~IRenderNodeStatusListener() {}
};

#pragma pack(push, 16)

//     Light volumes data

#define LIGHTVOLUME_MAXLIGHTS 16

struct SLightVolume
{
	struct SLightData
	{
		SLightData()
			: position(ZERO)
			, projectorDirection(ZERO)
			, color(ZERO)
			, radius(0.0f)
			, buldRadius(0.0f)
			, projectorCosAngle(0.0f) {}

		Vec3  position;
		Vec3  projectorDirection;
		Vec3  color;
		float radius;
		float buldRadius;
		float projectorCosAngle;
	};

	SLightVolume()
	{
		pData.reserve(LIGHTVOLUME_MAXLIGHTS);
	}

	typedef DynArray<SLightData> LightDataVector;

	DRX_ALIGN(16) LightDataVector pData;
};

#pragma pack(pop)

struct I3DEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(I3DEngineModule, "31bd20ff-1347-4f02-b923-c3f83ba73d84"_drx_guid);
};

//! Interface to the 3d Engine.
struct I3DEngine : public IProcess
{

	struct SObjectsStreamingStatus
	{
		i32 nReady;
		i32 nInProgress;
		i32 nTotal;
		i32 nActive;
		i32 nAllocatedBytes;
		i32 nMemRequired;
		i32 nMeshPoolSize; //!< In MB.
	};

	struct SStremaingBandwidthData
	{
		SStremaingBandwidthData()
		{
			memset(this, 0, sizeof(SStremaingBandwidthData));
		}
		float fBandwidthActual;
		float fBandwidthRequested;
	};

	enum eStreamingSubsystem
	{
		eStreamingSubsystem_Textures,
		eStreamingSubsystem_Objects,
		eStreamingSubsystem_Audio,
	};

	// <interfuscator:shuffle>
	//! Initializes the 3D Engine.
	virtual bool Init() = 0;

	//! Sets the path used to load levels.
	//! \param szFolderName Should contains the folder to be used.
	virtual void SetLevelPath(tukk szFolderName) = 0;

	virtual void PrepareOcclusion(const CCamera& rCamera) = 0;
	virtual void EndOcclusion() = 0;

	//! Load a level.
	//! Will load a level from the folder specified with SetLevelPath. If a
	//! level is already loaded, the resources will be deleted before.
	//! \param szFolderName - Name of the subfolder to load
	//! \param szMissionName - Name of the mission
	//! \return A boolean which indicate the result of the function; true if succeeded, false if failed.
	virtual bool LoadLevel(tukk szFolderName, tukk szMissionName) = 0;
	virtual bool InitLevelForEditor(tukk szFolderName, tukk szMissionName) = 0;

	//! Handles any work needed at start of new frame.
	//! \note Should be called for every frame.
	virtual void OnFrameStart() = 0;

	//! Pre-caches some resources need for rendering.
	//! Must be called after the game completely finishes loading the level.
	//! 3D engine uses it to pre-cache some resources needed for rendering.
	virtual void PostLoadLevel() = 0;

	//! Clears all rendering resources, all objects, characters and materials, voxels and terrain.
	//! \note Should always be called before LoadLevel, and also before loading textures from a script.
	virtual void UnloadLevel() = 0;

	//! Updates the 3D Engine.
	//! \note Should be called for every frame.
	virtual void Update() = 0;

	//! \note This is the camera which should be used for all Engine side culling (since e_camerafreeze allows easy debugging then)
	//! \note Only valid during RenderWorld(else the camera of the last frame is used)
	//! \return the Camera used for Rendering on 3DEngine Side, normaly equal to the view camera, except if frozen with e_camerafreeze
	virtual const CCamera& GetRenderingCamera() const = 0;
	virtual float          GetZoomFactor() const = 0;
	virtual float          IsZoomInProgress()  const = 0;

	//! Clear all per frame temp data used in SRenderingPass.
	virtual void Tick() = 0;

	//! Update all ShaderItems flags, only required after shaders were reloaded at runtime.
	virtual void UpdateShaderItems() = 0;

	//! Deletes the 3D Engine instance.
	virtual void Release() = 0;

	//! Draws the world.
	//! \param szDebugName Name that can be visualized for debugging purpose, must not be 0.
	virtual void RenderWorld(i32k nRenderFlags, const SRenderingPassInfo& passInfo, tukk szDebugName) = 0;

	//! Prepares for the world stream update, should be called before rendering.
	virtual void PreWorldStreamUpdate(const CCamera& cam) = 0;

	//! Performs the actual world streaming update. PreWorldStreamUpdate must be called before.
	virtual void WorldStreamUpdate() = 0;

	//! Shuts down the 3D Engine.
	virtual void ShutDown() = 0;

	//! Loads a static object from a CGF file.
	//! \param szFileName - CGF Filename - should not be 0 or ""
	//! \param szGeomName - Optional name of geometry inside CGF.
	//! \param ppSubObject - [Out]Optional Out parameter,Pointer to the
	//! \param nLoadingFlags - Zero or a bitwise combination of the flags from ELoadingFlags, defined in IMaterial.h, under the interface IMaterialUpr.
	//! \return A pointer to an object derived from IStatObj.
	virtual IStatObj* LoadStatObj(tukk szFileName, tukk szGeomName = NULL, /*[Out]*/ IStatObj::SSubObject** ppSubObject = NULL, bool bUseStreaming = true, u64 nLoadingFlags = 0) = 0;

	//! Finds a static object created from the given filename.
	//! \param szFileName - CGF Filename - should not be 0 or "".
	//! \return A pointer to an object derived from IStatObj.
	virtual IStatObj* FindStatObjectByFilename(tukk filename) = 0;

	virtual void      ResetCoverageBufferSignalVariables() = 0;

	//! Gets the amount of loaded objects.
	//! \return An integer representing the amount of loaded objects.
	virtual i32 GetLoadedObjectCount() { return 0; }

	//! Fills pObjectsArray with pointers to loaded static objects.
	//! if pObjectsArray is NULL only fills nCount parameter with amount of loaded objects.
	virtual void GetLoadedStatObjArray(IStatObj** pObjectsArray, i32& nCount) = 0;

	//! Gets stats on streamed objects.
	virtual void GetObjectsStreamingStatus(SObjectsStreamingStatus& outStatus) = 0;

	//! Gets stats on the streaming bandwidth requests from subsystems.
	//! \param subsystem Rhe streaming subsystem for which we want bandwidth data.
	//! \param outData Structure containing the bandwidth data for the subsystem requested.
	virtual void GetStreamingSubsystemData(i32 subsystem, SStremaingBandwidthData& outData) = 0;

	//! Registers an entity to be rendered.
	//! \param pEntity The entity to render.
	virtual void RegisterEntity(IRenderNode* pEntity) = 0;

	//! Selects an entity for debugging.
	//! \param pEntity - The entity to render.
	virtual void SelectEntity(IRenderNode* pEntity) = 0;

#ifndef _RELEASE
	enum EDebugDrawListAssetTypes
	{
		DLOT_ALL        = 0,
		DLOT_BRUSH      = BIT(0),
		DLOT_VEGETATION = BIT(1),
		DLOT_CHARACTER  = BIT(2),
		DLOT_STATOBJ    = BIT(3)
	};

	struct SObjectInfoToAddToDebugDrawList
	{
		tukk              pName;
		tukk              pClassName;
		tukk              pFileName;
		IRenderNode*             pRenderNode;
		u32                   numTris;
		u32                   numVerts;
		u32                   texMemory;
		u32                   meshMemory;
		EDebugDrawListAssetTypes type;
		const AABB*              pBox;
		const Matrix34*          pMat;
	};

	virtual void AddObjToDebugDrawList(SObjectInfoToAddToDebugDrawList& objInfo) = 0;
	virtual bool IsDebugDrawListEnabled() const = 0;
#endif

	//! Notices the 3D Engine to stop rendering a specified entity.
	//! \param pEntity - The entity to stop render
	virtual void UnRegisterEntityDirect(IRenderNode* pEntity) = 0;
	virtual void UnRegisterEntityAsJob(IRenderNode* pEnt) = 0;

	//! Add a water ripple to the scene.
	virtual void AddWaterRipple(const Vec3& vPos, float scale, float strength) = 0;

	//! \return whether a world pos is under water.
	virtual bool IsUnderWater(const Vec3& vPos) const = 0;

	//! \return whether ocean volume is visible or not.
	virtual void   SetOceanRenderFlags(u8 nFlags) = 0;
	virtual u8  GetOceanRenderFlags() const = 0;
	virtual u32 GetOceanVisiblePixelsCount() const = 0;

	//! Gets the closest walkable bottom z straight beneath the given reference position.
	//! \note This function will take into account both the global terrain elevation and local voxel (or other solid walkable object).
	//! \param referencePos - Position from where to start searching downwards.
	//! \param maxRelevantDepth - Max depth caller is interested in relative to referencePos (for ray casting performance reasons).
	//! \param objtypes - expects physics entity flags.  Use this to specify what object types make a valid bottom for you.
	//! \return A float value which indicate the global world z of the bottom level beneath the referencePos.
	//! \return If the referencePos is below terrain but not inside any voxel area BOTTOM_LEVEL_UNKNOWN is returned.
	virtual float GetBottomLevel(const Vec3& referencePos, float maxRelevantDepth, i32 objtypes) = 0;
	// A set of overloads for enabling users to use different sets of input params.  Basically, only
	// referencePos is mandatory.  The overloads as such don't need to be virtual but this seems to be
	// a purely virtual interface.
	virtual float GetBottomLevel(const Vec3& referencePos, float maxRelevantDepth = 10.0f) = 0;
	virtual float GetBottomLevel(const Vec3& referencePos, i32 objflags) = 0;

	//! Gets the ocean water level. Fastest option, always prefer is only ocean height required.
	//! \note This function will take into account just the global water level.
	//! \return A float value which indicate the water level. In case no water was
	//! \return found at the specified location, the value WATER_LEVEL_UNKNOWN will
	//! \return be returned.
	virtual float GetWaterLevel() = 0;

	//! Gets the closest walkable bottom z straight beneath the given reference position.
	//! Use with accurate query with caution - it is slow.
	//! \note This function will take into account both the global water level and any water volume present.
	//! \note Function is provided twice for performance with diff. arguments.
	//! \param pvPos - Desired position to inspect the water level
	//! \param pent - Pointer to return the physical entity to test against (optional)
	//! \return A float value which indicate the water level. In case no water was
	//! \return found at the specified location, the value WATER_LEVEL_UNKNOWN will
	//! \return be returned.
	virtual float GetWaterLevel(const Vec3* pvPos, IPhysicalEntity* pent = NULL, bool bAccurate = false) = 0;

	//! Gets the ocean water level for a specified position.
	//! Use with accurate query with caution - it is slow.
	//! \note This function only takes into account ocean water.
	//! \param pCurrPos - Position to check water level.
	//! \return A float value which indicate the water level.
	virtual float GetAccurateOceanHeight(const Vec3& pCurrPos) const = 0;

	//! Gets caustics parameters.
	//! \return A Vec4 value which constains:
	//! \return x = unused, y = distance attenuation, z = caustics multiplier, w = caustics darkening multiplier
	virtual Vec4 GetCausticsParams() const = 0;

	//! Gets ocean animation caustics parameters.
	//! \return A Vec4 value which constains: x = unused, y = height, z = depth, w = intensity
	virtual Vec4 GetOceanAnimationCausticsParams() const = 0;

	//! Gets ocean animation parameters.
	//! \return 2 Vec4s which constain:
	//!         0: x = ocean wind direction, y = wind speed, z = free, w = waves amount
	//!         1: x = waves size, y = free, z = free, w = free
	virtual void GetOceanAnimationParams(Vec4& pParams0, Vec4& pParams1) const = 0;

	//! Gets HDR setup parameters.
	virtual void GetHDRSetupParams(Vec4 pParams[5]) const = 0;

	//! Removes all particles and decals from the world.
	virtual void ResetParticlesAndDecals() = 0;

	//! Creates new decals on the walls, static objects, terrain and entities.
	//! \param Decal - Structure describing the decal effect to be applied
	virtual void CreateDecal(const DinrusXDecalInfo& Decal) = 0;

	//! Removes decals in a specified range.
	//! \param vAreaBox Specify the area in which the decals will be removed.
	//! \param pEntity  If not NULL will only delete decals attached to this entity.
	virtual void DeleteDecalsInRange(AABB* pAreaBox, IRenderNode* pEntity) = 0;

	//! Renders far trees/objects as sprites.
	//! It's a call back for renderer. It renders far trees/objects as sprites.
	//! \note Used by renderer, will be removed from here.
	virtual void DrawFarTrees(const SRenderingPassInfo& passInfo) = 0;

	//!< Used by renderer.
	virtual void GenerateFarTrees(const SRenderingPassInfo& passInfo) = 0;

	//! Sets the current outdoor ambient color.
	virtual void SetSkyColor(Vec3 vColor) = 0;

	//! Sets the current sun color.
	virtual void SetSunColor(Vec3 vColor) = 0;

	//! Sets the current sky brightening multiplier.
	virtual void SetSkyBrightness(float fMul) = 0;

	//! Gets the current sun/sky color relation.
	virtual float GetSunRel() const = 0;

	//! Sets current rain parameters.
	virtual void SetRainParams(const SRainParams& rainParams) = 0;

	//! Gets the validity and fills current rain parameters.
	virtual bool GetRainParams(SRainParams& rainParams) = 0;

	//! Sets current snow surface parameters.
	virtual void SetSnowSurfaceParams(const Vec3& vCenter, float fRadius, float fSnowAmount, float fFrostAmount, float fSurfaceFreezing) = 0;

	//! Gets current snow surface parameters.
	virtual bool GetSnowSurfaceParams(Vec3& vCenter, float& fRadius, float& fSnowAmount, float& fFrostAmount, float& fSurfaceFreezing) = 0;

	//! Sets current snow parameters.
	virtual void SetSnowFallParams(i32 nSnowFlakeCount, float fSnowFlakeSize, float fSnowFallBrightness, float fSnowFallGravityScale, float fSnowFallWindScale, float fSnowFallTurbulence, float fSnowFallTurbulenceFreq) = 0;

	//! Gets current snow parameters.
	virtual bool GetSnowFallParams(i32& nSnowFlakeCount, float& fSnowFlakeSize, float& fSnowFallBrightness, float& fSnowFallGravityScale, float& fSnowFallWindScale, float& fSnowFallTurbulence, float& fSnowFallTurbulenceFreq) = 0;

	//! Sets the view distance scale.
	//! \param fScale - may be between 0 and 1, 1.f = Unmodified view distance set by level designer, value of 0.5 will reduce it twice.
	//! \note This value will be reset automatically to 1 on next level loading.
	virtual void SetMaxViewDistanceScale(float fScale) = 0;

	//! Gets the view distance.
	//! \return A float value representing the maximum view distance.
	virtual float                GetMaxViewDistance(bool bScaled = true) = 0;

	virtual const SFrameLodInfo& GetFrameLodInfo() const = 0;
	virtual void                 SetFrameLodInfo(const SFrameLodInfo& frameLodInfo) = 0;

	//! Sets the fog color.
	virtual void SetFogColor(const Vec3& vFogColor) = 0;

	//! Gets the fog color.
	virtual Vec3 GetFogColor() = 0;

	//! Gets various sky light parameters.
	virtual void GetSkyLightParameters(Vec3& sunDir, Vec3& sunIntensity, float& Km, float& Kr, float& g, Vec3& rgbWaveLengths) = 0;

	//! Sets various sky light parameters.
	virtual void SetSkyLightParameters(const Vec3& sunDir, const Vec3& sunIntensity, float Km, float Kr, float g, const Vec3& rgbWaveLengths, bool forceImmediateUpdate = false) = 0;

	//! \return In logarithmic scale -4.0 .. 4.0
	virtual float GetLightsHDRDynamicPowerFactor() const = 0;

	//! \return true if tessellation is allowed for given render object.
	virtual bool IsTessellationAllowed(const CRenderObject* pObj, const SRenderingPassInfo& passInfo, bool bIgnoreShadowPass = false) const = 0;

	//! Allows to modify material on render nodes at run-time (make sure it is properly restored back).
	virtual void SetRenderNodeMaterialAtPosition(EERType eNodeType, const Vec3& vPos, IMaterial* pMat) = 0;

	//! Override the camera precache point with the requested position for the current round.
	virtual void OverrideCameraPrecachePoint(const Vec3& vPos) = 0;

	//! Begin streaming of meshes and textures for specified position, pre-cache stops after fTimeOut seconds.
	virtual i32  AddPrecachePoint(const Vec3& vPos, const Vec3& vDir, float fTimeOut = 3.f, float fImportanceFactor = 1.0f) = 0;
	virtual void ClearPrecachePoint(i32 id) = 0;
	virtual void ClearAllPrecachePoints() = 0;

	virtual void GetPrecacheRoundIds(i32 pRoundIds[MAX_STREAM_PREDICTION_ZONES]) = 0;

	virtual void TraceFogVolumes(const Vec3& worldPos, ColorF& fogVolumeContrib, const SRenderingPassInfo& passInfo) = 0;

	//! Gets the interpolated terrain elevation for a specified location.
	//! All x,y values are valid.
	//! \param x X coordinate of the location.
	//! \param y Y coordinate of the location.
	//! \return A float which indicate the elevation level.
	virtual float GetTerrainElevation(float x, float y) = 0;

	//! Gets the terrain elevation for a specified location.
	//! Only values between 0 and WORLD_SIZE.
	//! \param x X coordinate of the location.
	//! \param y Y coordinate of the location.
	//! \return A float which indicate the elevation level.
	virtual float GetTerrainZ(float x, float y) = 0;

	//! Gets the terrain hole flag for a specified location.
	//! Only values between 0 and WORLD_SIZE.
	//! \param x - X coordinate of the location.
	//! \param y - Y coordinate of the location.
	//! \return A bool which indicate is there hole or not.
	virtual bool GetTerrainHole(float x, float y) = 0;

	//! Gets the terrain surface normal for a specified location.
	//! \param vPos.x - X coordinate of the location.
	//! \param vPos.y - Y coordinate of the location.
	//! \param vPos.z - ignored.
	//! \return A terrain surface normal.
	virtual Vec3 GetTerrainSurfaceNormal(Vec3 vPos) = 0;

	//! Gets the unit size of the terrain.
	//! The value should currently be 2.
	//! \return A i32 value representing the terrain unit size in meters.
	virtual float GetHeightMapUnitSize() = 0;

	//! Gets the size of the terrain.
	//! The value should be 2048 by default.
	//! \return An i32 representing the terrain size in meters.
	virtual i32 GetTerrainSize() = 0;

	//! Gets the size of the terrain sectors.
	//! The value should be 64 by default.
	//! \return An i32 representing the size of a sector in meters.
	virtual i32 GetTerrainSectorSize() = 0;

	// Internal functions, mostly used by the editor, which won't be documented for now.

	// Summary:
	//		Places object at specified position (for editor)
	//	virtual bool AddStaticObject(i32 nObjectID, const Vec3 & vPos, const float fScale, u8 ucBright=255) = 0;
	// Summary:
	//		Removes static object from specified position (for editor)
	//	virtual bool RemoveStaticObject(i32 nObjectID, const Vec3 & vPos) = 0;
	// Summary:
	//		On-demand physicalization of a static object
	//	virtual bool PhysicalizeStaticObject(uk pForeignData,i32 iForeignData,i32 iForeignFlags) = 0;
	// Summary:
	//		Removes all static objects on the map (for editor)
	virtual void RemoveAllStaticObjects() = 0;
	// Summary:
	//		Allows to set terrain surface type id for specified point in the map (for editor)
	virtual void SetTerrainSurfaceType(i32 x, i32 y, i32 nType) = 0; // from 0 to 6 - sur type ( 7 = hole )

	// Summary:
	//		Returns true if game modified terrain hight map since last update by editor
	virtual bool IsTerrainHightMapModifiedByGame() = 0;
	// Summary:
	//		Updates hight map max height (in meters)
	virtual void SetHeightMapMaxHeight(float fMaxHeight) = 0;

	// Summary:
	//		Sets terrain sector texture id, and disable streaming on this sector
	virtual void SetTerrainSectorTexture(i32k nTexSectorX, i32k nTexSectorY, u32 textureId) = 0;

	// Summary:
	//		Returns size of smallest terrain texture node (last leaf) in meters
	virtual i32 GetTerrainTextureNodeSizeMeters() = 0;

	// Arguments:
	//   nLayer - 0=diffuse texture, 1=occlusionmap
	// Return value:
	//   an integer value representing the size of terrain texture node in pixels
	virtual i32 GetTerrainTextureNodeSizePixels(i32 nLayer) = 0;

	// Summary:
	//		Sets group parameters
	virtual bool SetStatInstGroup(i32 nGroupId, const IStatInstGroup& siGroup) = 0;

	// Summary:
	//		Gets group parameters
	virtual bool GetStatInstGroup(i32 nGroupId, IStatInstGroup& siGroup) = 0;

	// Summary:
	//		Sets burbed out flag
	virtual void SetTerrainBurnedOut(i32 x, i32 y, bool bBurnedOut) = 0;

	// Summary:
	//		Gets burbed out flag
	virtual bool IsTerrainBurnedOut(i32 x, i32 y) = 0;

	//! Notifies of an explosion, and maybe creates an hole in the terrain.
	//! This function should usually make sure that no static objects are near before making the hole.
	//! \param vPos - Position of the explosion
	//! \param fRadius - Radius of the explosion
	//! \param bDeformTerrain - Allow to deform the terrain
	virtual void OnExplosion(Vec3 vPos, float fRadius, bool bDeformTerrain = true) = 0;

	//! Sets the physics material enumerator.
	//! \param pPhysMaterialEnumerator The physics material enumarator to set.
	virtual void SetPhysMaterialEnumerator(IPhysMaterialEnumerator* pPhysMaterialEnumerator) = 0;

	//! Gets the physics material enumerator.
	//! \return A pointer to an IPhysMaterialEnumerator derived object.
	virtual IPhysMaterialEnumerator* GetPhysMaterialEnumerator() = 0;

	// Summary:
	//	 Loads environment settings for specified mission
	virtual void LoadMissionDataFromXMLNode(tukk szMissionName) = 0;

	virtual void LoadEnvironmentSettingsFromXML(XmlNodeRef pInputNode) = 0;

	// Summary:
	//	 Loads detail texture and detail object settings from XML doc (load from current LevelData.xml if pDoc is 0)
	virtual void LoadTerrainSurfacesFromXML(XmlNodeRef pDoc, bool bUpdateTerrain) = 0;

	//! Applies physics in a specified area
	//! Physics applied to the area will apply to vegetations and allow it to move/blend.
	//! \param vPos - Center position to apply physics
	//! \param fRadius - Radius which specify the size of the area to apply physics
	//! \param fAmountOfForce - The amount of force, should be at least of 1.0f
	virtual void ApplyForceToEnvironment(Vec3 vPos, float fRadius, float fAmountOfForce) = 0;

	// Set direction to the sun.
	//  virtual void SetSunDir( const Vec3& vNewSunDir ) = 0;.
	//! \internal
	//! Return non-normalized direction to the sun.
	virtual Vec3 GetSunDir()  const = 0;

	//! \internal
	//! Return normalized direction to the sun.
	virtual Vec3 GetSunDirNormalized()  const = 0;

	//! \internal
	//! \return realtime (updated every frame with real sun position) normalized direction to the scene.
	virtual Vec3 GetRealtimeSunDirNormalized()  const = 0;

	//! \internal
	//! Internal function used by the 3d engine.
	//! \return lighting level for this point.
	virtual Vec3 GetAmbientColorFromPosition(const Vec3& vPos, float fRadius = 1.f) = 0;

	//! \internal
	//! Internal function used by 3d engine and renderer.
	//! Gets distance to the sector containig ocean water
	virtual float GetDistanceToSectorWithWater() = 0;

	//! Gets the environment ambient color.
	//! \note Should have been specified in the editor.
	//! \return An rgb value contained in a Vec3 object.
	virtual Vec3 GetSkyColor() const = 0;

	//! Gets the sun color.
	//! \note Should have been specified in the editor.
	//! \return An rgb value contained in a Vec3 object.
	virtual Vec3 GetSunColor() const = 0;

	//! Retrieves the current sky brightening multiplier.
	//! \return Scalar value
	virtual float GetSkyBrightness() const = 0;

	//! Retrieves the current SSAO multiplier.
	//! \return scalar value
	virtual float GetSSAOAmount() const = 0;

	//! Retrieves the current SSAO contrast multiplier.
	//! \return scalar value
	virtual float GetSSAOContrast() const = 0;

	//! Retrieves the current GI multiplier.
	//! \return scalar value
	virtual float GetGIAmount() const = 0;

	//! Retrieves terrain texture multiplier.
	//! \return Scalar value
	virtual float GetTerrainTextureMultiplier() const = 0;

	//  check object visibility taking into account portals and terrain occlusion test
	//  virtual bool IsBoxVisibleOnTheScreen(const Vec3 & vBoxMin, const Vec3 & vBoxMax, OcclusionTestClient * pOcclusionTestClient = NULL)=0;
	//  check object visibility taking into account portals and terrain occlusion test
	//  virtual bool IsSphereVisibleOnTheScreen(const Vec3 & vPos, const float fRadius, OcclusionTestClient * pOcclusionTestClient = NULL)=0;

	//mat: todo

	//! Frees entity render info.
	virtual void FreeRenderNodeState(IRenderNode* pEntity) = 0;

	//! Adds the level's path to a specified filename.
	//! \param szFileName The filename for which we need to add the path.
	//! \return Full path for the filename; including the level path and the filename appended after.
	virtual tukk GetLevelFilePath(tukk szFileName) = 0;

	//! Displays statistic on the 3d Engine.
	//! \param fTextPosX X position for the text.
	//! \param fTextPosY Y position for the text.
	//! \param fTextStepY Amount of pixels to distance each line.
	//! \param bEnhanced false=normal, true=more interesting information.
	virtual void DisplayInfo(float& fTextPosX, float& fTextPosY, float& fTextStepY, const bool bEnhanced) = 0;

	//! Draws text right aligned at the y pixel precision.
	virtual void DrawTextRightAligned(const float x, const float y, tukk format, ...) PRINTF_PARAMS(4, 5) = 0;
	virtual void DrawTextRightAligned(const float x, const float y, const float scale, const ColorF& color, tukk format, ...) PRINTF_PARAMS(6, 7) = 0;

	//! Enables or disables portal at a specified position.
	//! \param vPos Position to place the portal
	//! \param bActivate Set to true in order to enable the portal, or to false to disable
	//! \param szEntityName
	virtual void ActivatePortal(const Vec3& vPos, bool bActivate, tukk szEntityName) = 0;
	virtual void ActivateOcclusionAreas(IVisAreaTestCallback* pTest, bool bActivate) = 0;

	//! \internal
	//! Counts memory usage
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! \internal
	//! Counts resource memory usage.
	//! \param cstAABB - Use the whole level AABB if you want to grab the resources from the whole level. For height level, use something BIG (ie: +-FLT_MAX).
	virtual void GetResourceMemoryUsage(IDrxSizer* pSizer, const AABB& cstAABB) = 0;

	//! Creates a new VisArea.
	//! \return A pointer to a newly created VisArea object.
	virtual IVisArea* CreateVisArea(uint64 visGUID) = 0;

	//! Deletes a VisArea.
	//! \param pVisArea - A pointer to the VisArea to delete.
	virtual void DeleteVisArea(IVisArea* pVisArea) = 0;

	//mat: todo

	//! Updates the VisArea
	virtual void UpdateVisArea(IVisArea* pArea, const Vec3* pPoints, i32 nCount, tukk szName, const SVisAreaInfo& info, bool bReregisterObjects) = 0;

	//! Determines if two VisAreas are connected.
	//! Used to determine if a sound is potentially hearable between two VisAreas.
	//! \param pArea1 A pointer to a VisArea.
	//! \param pArea2 A pointer to a VisArea.
	//! \param nMaxRecursion Maximum number of recursions to be done.
	//! \param bSkipDisabledPortals Indicate if disabled portals should be skipped.
	//! \return A boolean value set to true if the two VisAreas are connected, else false will be returned.
	virtual bool IsVisAreasConnected(IVisArea* pArea1, IVisArea* pArea2, i32 nMaxRecursion = 1, bool bSkipDisabledPortals = true) = 0;

	//! Creates a ClipVolume.
	//! \return A pointer to a newly created ClipVolume object.
	virtual IClipVolume* CreateClipVolume() = 0;

	//! Deletes a ClipVolume.
	//! \param pClipVolume - A pointer to the ClipVolume to delete.
	virtual void DeleteClipVolume(IClipVolume* pClipVolume) = 0;

	//! Updates a ClipVolume.
	//! \param pClipVolume Pointer to volume that needs updating.
	//! \param pRenderMesh Pointer to new render mesh.
	//! \param worldTM Updated world transform.
	//! \param szName Updated ClipVolume name.
	virtual void UpdateClipVolume(IClipVolume* pClipVolume, _smart_ptr<IRenderMesh> pRenderMesh, IBSPTree3D* pBspTree, const Matrix34& worldTM, u8 viewDistRatio, bool bActive, u32 flags, tukk szName) = 0;

	//mat: todo

	//! Creates instance of IRenderNode object with specified type.
	virtual IRenderNode* CreateRenderNode(EERType type) = 0;

	//! Delete RenderNode object.
	virtual void DeleteRenderNode(IRenderNode* pRenderNode) = 0;

	//! Set global wind vector.
	virtual void SetWind(const Vec3& vWind) = 0;

	//! Gets wind direction and force, averaged within a box.
	virtual Vec3 GetWind(const AABB& box, bool bIndoors) const = 0;

	//! Gets the global wind vector.
	virtual Vec3 GetGlobalWind(bool bIndoors) const = 0;

	//! Gets wind direction and forace at the sample points provided.
	//! \note The positions defining the samples will be overwritten with the accumulated wind influences.
	virtual bool SampleWind(Vec3* pSamples, i32 nSamples, const AABB& volume, bool bIndoors) const = 0;

	//! Gets the VisArea which is present at a specified point.
	//! \return VisArea containing point, if any, 0 otherwise.
	virtual IVisArea* GetVisAreaFromPos(const Vec3& vPos) = 0;

	//! Tests for intersection against Vis Areas.
	//! \param[in] box Volume to test for intersection.
	//! \paran[out] pNodeCache Optional, set to a cached pointer for quicker calls to ClipToVisAreas.
	//! \return Whether box intersects any vis areas.
	virtual bool IntersectsVisAreas(const AABB& box, uk * pNodeCache = 0) = 0;

	//! Clips geometry against the boundaries of VisAreas.
	//! \param pInside: Vis Area to clip inside of. If 0, clip outside all Vis Areas.
	//! \return True if it was clipped.
	virtual bool ClipToVisAreas(IVisArea* pInside, Sphere& sphere, Vec3 const& vNormal, uk pNodeCache = 0) = 0;

	//! Enables or disables ocean rendering.
	//! \param bOcean - Will enable or disable the rendering of ocean.
	virtual void EnableOceanRendering(bool bOcean) = 0;

	//! Creates a new light source.
	//! \return Pointer to newly created light or -1 if it fails.
	virtual struct ILightSource* CreateLightSource() = 0;

	//! Deletes a light.
	//! \param pLightSource Pointer to the light.
	virtual void DeleteLightSource(ILightSource* pLightSource) = 0;

	//! Gives access to the list holding all static light sources.
	//! \return An array holding all the SRenderLight pointers.
	virtual const PodArray<SRenderLight*>* GetStaticLightSources() = 0;
	virtual const PodArray<ILightSource*>* GetLightEntities() = 0;

	//! Gives access to list holding all lighting volumes.
	//! \return An array holding all the SLightVolume pointers.
	virtual void GetLightVolumes(threadID nThreadID, SLightVolume*& pLightVols, u32& nNumVols) = 0;

	//! Reload the heightmap.
	//! Reloading the heightmap will resets all decals and particles.
	//! \note In future will restore deleted vegetations
	//! \return true on success, false otherwise.
	virtual bool RestoreTerrainFromDisk() = 0;

	//! \internal
	//! Tmp.
	virtual tukk GetFilePath(tukk szFileName) { return GetLevelFilePath(szFileName); }

	// Post-processing effects interfaces.
	virtual void  SetPostEffectParam(tukk pParam, float fValue, bool bForceValue = false) const = 0;
	virtual void  SetPostEffectParamVec4(tukk pParam, const Vec4& pValue, bool bForceValue = false) const = 0;
	virtual void  SetPostEffectParamString(tukk pParam, tukk pszArg) const = 0;

	virtual void  GetPostEffectParam(tukk pParam, float& fValue) const = 0;
	virtual void  GetPostEffectParamVec4(tukk pParam, Vec4& pValue) const = 0;
	virtual void  GetPostEffectParamString(tukk pParam, tukk & pszArg) const = 0;

	virtual i32 GetPostEffectID(tukk pPostEffectName) = 0;

	virtual void  ResetPostEffects(bool bOnSpecChange = false) const = 0;

	virtual void  SetShadowsGSMCache(bool bCache) = 0;
	virtual void  SetCachedShadowBounds(const AABB& shadowBounds, float fAdditionalCascadesScale) = 0;
	virtual void  SetRecomputeCachedShadows(uint nUpdateStrategy = 0) = 0;
	virtual void  InvalidateShadowCacheData() = 0;

	//! Physicalizes area if not physicalized yet.
	virtual void CheckPhysicalized(const Vec3& vBoxMin, const Vec3& vBoxMax) = 0;

	//! In debug mode, check memory heap and makes assert, do nothing in release
	virtual void CheckMemoryHeap() = 0;

	//! Closes terrain texture file handle and allows to replace/update it.
	virtual void CloseTerrainTextureFile() = 0;

	//! Removes all decals attached to specified entity.
	virtual void DeleteEntityDecals(IRenderNode* pEntity) = 0;

	//! Finishes objects geometery generation/loading.
	virtual void CompleteObjectsGeometry() = 0;

	//! Disables CGFs unloading.
	virtual void LockCGFResources() = 0;

	//! Enables CGFs unloading (this is default state), this function will also release all not used CGF's.
	virtual void UnlockCGFResources() = 0;

	//! Creates static object containing empty IndexedMesh.
	virtual IStatObj* CreateStatObj() = 0;
	virtual IStatObj* CreateStatObjOptionalIndexedMesh(bool createIndexedMesh) = 0;

	//! Creates the instance of the indexed mesh.
	virtual IIndexedMesh* CreateIndexedMesh() = 0;

	//! Updates rendering mesh in the stat obj associated with pPhysGeom.
	//! \note Creates or clones the object if necessary.
	virtual IStatObj* UpdateDeformableStatObj(IGeometry* pPhysGeom, bop_meshupdate* pLastUpdate = 0, IFoliage* pSrcFoliage = 0) = 0;

	//! Saves/loads state of engine objects.
	virtual void SerializeState(TSerialize ser) = 0;

	//! Cleanups after save/load.
	virtual void PostSerialize(bool bReading) = 0;

	//! Retrieve pointer to the material i/o interface.
	virtual IMaterialHelpers& GetMaterialHelpers() = 0;

	//! Retrieve pointer to the material manager interface.
	virtual IMaterialUpr* GetMaterialUpr() = 0;

	//////////////////////////////////////////////////////////////////////////
	// CGF Loader.
	//////////////////////////////////////////////////////////////////////////
	//! Creates a chunkfile content instance.
	//! \return NULL if the memory for the instance could not be allocated.
	virtual CContentCGF* CreateChunkfileContent(tukk filename) = 0;

	//! Deletes the chunkfile content instance.
	virtual void ReleaseChunkfileContent(CContentCGF*) = 0;

	//! Loads the contents of a chunkfile into the given CContentCGF.
	//! \return true on success, false on error.
	virtual bool LoadChunkFileContent(CContentCGF* pCGF, tukk filename, bool bNoWarningMode = false, bool bCopyChunkFile = true) = 0;

	//! Loads the contents of a chunkfile into the given CContentCGF.
	//! \return true on success, false on error.
	virtual bool LoadChunkFileContentFromMem(CContentCGF* pCGF, ukk pData, size_t nDataLen, u32 nLoadingFlags, bool bNoWarningMode = false, bool bCopyChunkFile = true) = 0;

	//! Creates ChunkFile.
	virtual IChunkFile* CreateChunkFile(bool bReadOnly = false) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Chunk file writer.
	//////////////////////////////////////////////////////////////////////////
	enum EChunkFileFormat
	{
		eChunkFileFormat_0x745,
		eChunkFileFormat_0x746,
	};
	virtual ChunkFile::IChunkFileWriter* CreateChunkFileWriter(EChunkFileFormat eFormat, IDrxPak* pPak, tukk filename) const = 0;
	virtual void                         ReleaseChunkFileWriter(ChunkFile::IChunkFileWriter* p) const = 0;

	//////////////////////////////////////////////////////////////////////////

	//! \return Interface to terrain engine.
	virtual ITerrain* GetITerrain() = 0;

	//! Creates terrain engine.
	virtual ITerrain* CreateTerrain(const STerrainInfo& TerrainInfo) = 0;

	//! Deletes terrain.
	virtual void DeleteTerrain() = 0;

	//! \return Interface to visarea manager.
	virtual IVisAreaUpr* GetIVisAreaUpr() = 0;

	//! \return Interface to the mergedmeshes subsystem.
	virtual IMergedMeshesUpr* GetIMergedMeshesUpr() = 0;

	//! \return Amount of light affecting a point in space inside a specific range (0 means no light affecting,
	//!         1 is completely affected by light). Use accurate parameter for a more expensive but with higher accuracy computation.
	virtual float GetLightAmountInRange(const Vec3& pPos, float fRange, bool bAccurate = 0) = 0;

	//! Places camera into every visarea or every manually set pre-cache points and render the scenes.
	virtual void PrecacheLevel(bool bPrecacheAllVisAreas, Vec3* pPrecachePoints, i32 nPrecachePointsNum) = 0;

	//! Proposes 3dengine to load on next frame all shaders and textures synchronously.
	virtual void ProposeContentPrecache() = 0;

	//! \return TOD interface.
	virtual ITimeOfDay* GetTimeOfDay() = 0;

	//! \return SkyBox material.
	virtual IMaterial* GetSkyMaterial() = 0;

	//! Sets SkyBox Material.
	virtual void SetSkyMaterial(IMaterial* pSkyMat) = 0;

	//! Sets global 3d engine parameter.
	virtual void SetGlobalParameter(E3DEngineParameter param, const Vec3& v) = 0;
	void         SetGlobalParameter(E3DEngineParameter param, float val) { SetGlobalParameter(param, Vec3(val, 0, 0)); };

	//! Retrieves global 3d engine parameter.
	virtual void                     GetGlobalParameter(E3DEngineParameter param, Vec3& v) = 0;
	float                            GetGlobalParameter(E3DEngineParameter param) { Vec3 v(0, 0, 0); GetGlobalParameter(param, v); return v.x; };

	virtual void                     SetShadowMode(EShadowMode shadowMode) = 0;
	virtual EShadowMode              GetShadowMode() const = 0;
	virtual void                     AddPerObjectShadow(IShadowCaster* pCaster, float fConstBias, float fSlopeBias, float fJitter, const Vec3& vBBoxScale, uint nTexSize) = 0;
	virtual void                     RemovePerObjectShadow(IShadowCaster* pCaster) = 0;
	virtual struct SPerObjectShadow* GetPerObjectShadow(IShadowCaster* pCaster) = 0;
	virtual void                     GetCustomShadowMapFrustums(struct ShadowMapFrustum**& arrFrustums, i32& nFrustumCount) = 0;

	//! Saves pStatObj to a stream.
	//! \note Full mesh for generated ones, path/geom otherwise.
	virtual i32 SaveStatObj(IStatObj* pStatObj, TSerialize ser) = 0;

	//! Loads statobj from a stream
	virtual IStatObj* LoadStatObj(TSerialize ser) = 0;

	//! Removes references to RenderMesh
	virtual void OnRenderMeshDeleted(IRenderMesh* pRenderMesh) = 0;

	//! Removes references to IEntity
	virtual void OnEntityDeleted(struct IEntity* pEntity) = 0;

	//! Used to highlight an object under the reticule.
	virtual void DebugDraw_UpdateDebugNode() = 0;

	//! Used by editor during AO computations, deprecated
	virtual bool RayObjectsIntersection2D(Vec3 vStart, Vec3 vEnd, Vec3& vHitPoint, EERType eERType) { return false; }

	//! Used by editor during object alignment
	virtual bool RenderMeshRayIntersection(IRenderMesh* pRenderMesh, SRayHitInfo& hitInfo, IMaterial* pCustomMtl = 0) = 0;

	// pointer to ISegmentsUpr interface
	//! Warning: deprecated Segmented World implementation is not supported by drx3D anymore
	virtual ISegmentsUpr* GetSegmentsUpr()                                   { return nullptr; }
	virtual void              SetSegmentsUpr(ISegmentsUpr* pSegmentsUpr) {}

	//! \return true if segmented world is performing an operation (load/save/move/etc).
	//! Warning: deprecated Segmented World implementation is not supported by drx3D anymore
	virtual bool IsSegmentOperationInProgress()              { return false;  }
	virtual void SetSegmentOperationInProgress(bool bActive) {}

	//! Call function 2 times (first to get the size then to fill in the data)
	//! \param pObjects 0 if only the count is required
	//! \return Count returned.
	virtual u32 GetObjectsByType(EERType objType, IRenderNode** pObjects = 0) = 0;
	virtual u32 GetObjectsByTypeInBox(EERType objType, const AABB& bbox, IRenderNode** pObjects = 0, uint64 dwFlags = ~0) = 0;
	virtual u32 GetObjectsInBox(const AABB& bbox, IRenderNode** pObjects = 0) = 0;
	virtual u32 GetObjectsByFlags(uint dwFlag, IRenderNode** pObjects = 0) = 0;

	//! Called from editor whenever an object is modified by the user.
	virtual void        OnObjectModified(IRenderNode* pRenderNode, IRenderNode::RenderFlagsType dwFlags) = 0;

	virtual void        FillDebugFPSInfo(SDebugFPSInfo&) = 0;

	virtual void        SetTerrainLayerBaseTextureData(i32 nLayerId, byte* pImage, i32 nDim, tukk nImgFileName, IMaterial* pMat, float fBr, float fTiling, i32 nDetailSurfTypeId, float fTilingDetail, float fSpecularAmount, float fSortOrder, ColorF layerFilterColor, float fUseRemeshing, bool bShowSelection) = 0;

	virtual bool        IsAreaActivationInUse() = 0;

	virtual tukk GetVoxelEditOperationName(EVoxelEditOperation eOperation) = 0;

	//! Gives 3dengine access to original and most precise heighmap data in the editor
	virtual void                     SetEditorHeightmapCallback(IEditorHeightmap* pCallBack) = 0;

	virtual PodArray<SRenderLight*>* GetDynamicLightSources() = 0;

	virtual IParticleUpr*        GetParticleUpr() = 0;

	virtual IOpticsUpr*          GetOpticsUpr() = 0;

	//! Syncs and performs outstanding operations for the Asyncrhon ProcessStreaming Update
	virtual void SyncProcessStreamingUpdate() = 0;

	//! Set Callback for Editor to store additional information in Minimap tool.
	virtual void SetScreenshotCallback(IScreenshotCallback* pCallback) = 0;

	//! Register or unregister a call back for render node status updates
	virtual void RegisterRenderNodeStatusListener(IRenderNodeStatusListener* pListener, EERType renderNodeType) = 0;
	virtual void UnregisterRenderNodeStatusListener(IRenderNodeStatusListener* pListener, EERType renderNodeType) = 0;

	//! Show/Hide objects by layer (useful for streaming and performance).
	virtual void ActivateObjectsLayer(u16 nLayerId, bool bActivate, bool bPhys, bool bObjects, bool bStaticLights, tukk pLayerName, IGeneralMemoryHeap* pHeap = NULL, bool bCheckLayerActivation = true) = 0;

	//! Inform layer system about object aabb change
	virtual void UpdateObjectsLayerAABB(IRenderNode* pEnt) = 0;

	//! Get object layer memory usage
	virtual void GetLayerMemoryUsage(u16 nLayerId, IDrxSizer* pSizer, i32* pNumBrushes, i32* pNumDecals) const = 0;

	//! Collect layer ID's to skip loading objects from these layers, e.g. to skip console specific layers.
	virtual void SkipLayerLoading(u16 nLayerId, bool bClearList) = 0;

	//! Activate streaming of character and all sub-components.
	virtual void PrecacheCharacter(IRenderNode* pObj, const float fImportance, ICharacterInstance* pCharacter, IMaterial* pSlotMat, const Matrix34& matParent, const float fEntDistance, const float fScale, i32 nMaxDepth, bool bForceStreamingSystemUpdate, const SRenderingPassInfo& passInfo) = 0;

	//! Activate streaming of render node and all sub-components.
	virtual void                          PrecacheRenderNode(IRenderNode* pObj, float fEntDistanceReal) = 0;

	virtual IDeferredPhysicsEventUpr* GetDeferredPhysicsEventUpr() = 0;

	//! Return true if terrain texture streaming takes place.
	virtual bool IsTerrainTextureStreamingInProgress() = 0;
	virtual void SetStreamableListener(IStreamedObjectListener* pListener) = 0;

	//! Following functions are used by SRenderingPassInfo.
	virtual CCamera* GetRenderingPassCamera(const CCamera& rCamera) = 0;

	virtual i32      GetZoomMode() const = 0;
	virtual float    GetPrevZoomFactor() = 0;
	virtual void     SetZoomMode(i32 nZoomMode) = 0;
	virtual void     SetPrevZoomFactor(float fZoomFactor) = 0;

	//! LiveCreate.
	virtual void SaveInternalState(struct IDataWriteStream& writer, const AABB& filterArea, const bool bTerrain, u32k objectMask) = 0;
	virtual void LoadInternalState(struct IDataReadStream& reader, u8k* pVisibleLayersMasks, u16k* pLayerIdTranslation) = 0;

	virtual void OnCameraTeleport() = 0;

#if defined(FEATURE_SVO_GI)

	struct SAnalyticalOccluder
	{
		enum AnalyticalOccluderType
		{
			eCapsule = 0,
			eOBB,
			eCylinder,
			eOBB_Hard,
			eCylinder_Hard
		};

		Vec3 v0;

		union
		{
			float e0;
			float radius;
		};

		Vec3  v1;
		float e1;
		Vec3  v2;
		float e2;
		Vec3  c;
		float type;
	};

	struct SSvoStaticTexInfo
	{
		SSvoStaticTexInfo()
		{
			ZeroStruct(*this);
		}

		// SVO data pools
		_smart_ptr<ITexture> pTexTree;
		_smart_ptr<ITexture> pTexOpac;
		_smart_ptr<ITexture> pTexTris;
		_smart_ptr<ITexture> pTexRgb0;
		_smart_ptr<ITexture> pTexRgb1;
		_smart_ptr<ITexture> pTexDynl;
		_smart_ptr<ITexture> pTexRgb2;
		_smart_ptr<ITexture> pTexRgb3;
		_smart_ptr<ITexture> pTexRgb4;
		_smart_ptr<ITexture> pTexNorm;
		_smart_ptr<ITexture> pTexAldi;

		// mesh tracing data atlases
		_smart_ptr<ITexture> pTexTriA;
		_smart_ptr<ITexture> pTexTexA;
		_smart_ptr<ITexture> pTexIndA;

		_smart_ptr<ITexture> pGlobalSpecCM;

		float                fGlobalSpecCM_Mult;
		i32                  nTexDimXY;
		i32                  nTexDimZ;
		i32                  nBrickSize;
		bool                 bSvoReady;
		bool                 bSvoFreeze;
		Sphere               helperInfo;

	#define SVO_MAX_PORTALS 8
		Vec4 arrPortalsPos[SVO_MAX_PORTALS];
		Vec4 arrPortalsDir[SVO_MAX_PORTALS];

	#define SVO_MAX_ANALYTICAL_OCCLUDERS 32
		SAnalyticalOccluder arrAnalyticalOccluders[2][SVO_MAX_ANALYTICAL_OCCLUDERS];

		Vec3                vSkyColorTop;
		Vec3                vSkyColorBottom;
		Vec4                vSvoOriginAndSize;
	};

	struct SLightTI
	{
		Vec4            vPosR;
		Vec4            vDirF;
		Vec4            vCol;
		float           fSortVal;
		class ITexture* pCM;
	};

	virtual bool GetSvoStaticTextures(I3DEngine::SSvoStaticTexInfo& svoInfo, PodArray<I3DEngine::SLightTI>* pLightsTI_S, PodArray<I3DEngine::SLightTI>* pLightsTI_D) = 0;

	struct SSvoNodeInfo
	{
		AABB wsBox;
		AABB tcBox;
		i32  nAtlasOffset;
	};

	virtual void GetSvoBricksForUpdate(PodArray<SSvoNodeInfo>& arrNodeInfo, float fNodeSize, PodArray<SVF_P3F_C4B_T2F>* pVertsOut) = 0;
	virtual bool IsSvoReady(bool testPostponed) const = 0;
	virtual i32  GetSvoCompiledData(IDrxArchive* pArchive) = 0;

#endif

#if defined(USE_GEOM_CACHES)
	//! Loads a geometry cache from a CAX file.
	//! \param szFileName CAX Filename - should not be 0 or "".
	//! \return A pointer to an object derived from IGeomCache.
	virtual IGeomCache* LoadGeomCache(tukk szFileName) = 0;

	//! Finds a geom cache created from the given filename.
	//! \param szFileName CAX Filename - should not be 0 or "".
	//! \return A pointer to an object derived from IGeomCache.
	virtual IGeomCache* FindGeomCacheByFilename(tukk szFileName) = 0;
#endif

	//! Loads a designer object from a stream of _decoded_ binary <mesh> node (Base64Decode).
	//! \param szBinaryStream - decoded stream + size.
	virtual IStatObj* LoadDesignerObject(i32 nVersion, tukk szBinaryStream, i32 size) = 0;

	// </interfuscator:shuffle>
};

//==============================================================================================

//! \cond INTERNAL
//! Types of binary files used by 3dengine.
enum EFileTypes
{
	eTerrainTextureFile = 100,
};

#define FILEVERSION_TERRAIN_TEXTURE_FILE 9

//! Common header for binary files used by 3dengine.
struct SCommonFileHeader
{
	void Set(u16 t, u16 v)   { drx_strcpy(signature, "DRX"); file_type = (u8)t; version = v; }
	bool Check(u16 t, u16 v) { return strcmp(signature, "DRX") == 0 && t == file_type && v == version; }

	char   signature[4];                //!< File signature, should be "DRX ".
	u8  file_type;                   //!< File type.
	u8  flags;                       //!< File common flags.
	u16 version;                     //!< File version.

	AUTO_STRUCT_INFO;
};

//! Sub header for terrain texture file.
//! \note "locally higher texture resolution" following structure can be removed (as well in autotype)
struct STerrainTextureFileHeader_old
{
	u16 nSectorSizeMeters;
	u16 nLodsNum;
	u16 nLayerCount;              //!< STerrainTextureLayerFileHeader count following (also defines how may layers are interleaved) 1/2.
	u16 nReserved;

	AUTO_STRUCT_INFO;
};

#define TTFHF_AO_DATA_IS_VALID 1
#define TTFHF_BIG_ENDIAN       2

//! Subheader for terrain texture file
struct STerrainTextureFileHeader
{
	u16 nLayerCount;                 //!< STerrainTextureLayerFileHeader count following (also defines how may layers are interleaved) 1/2.
	u16 dwFlags;
	float  fBrMultiplier;

	AUTO_STRUCT_INFO;
};

//! Layer header for terrain texture file (for each layer)
struct STerrainTextureLayerFileHeader
{
	u16      nSectorSizePixels;
	u16      nReserved;          //!< Ensure padding and for later usage.
	ETEX_Format eTexFormat;         //!< Typically eTF_BC3.
	u32      nSectorSizeBytes;   //!< Redundant information for more convenient loading code.

	AUTO_STRUCT_INFO;
};

#pragma pack(pop)

#include <drx3D/CoreX/Math/Drx_Camera.h>

//! Class to wrap a special counter used to presort SRendItems.
//! This is used to fix random ordering introduced by parallelization of parts of the 3DEngine.
struct SRendItemSorter
{
	friend struct SRenderingPassInfo;

	//! Deferred PreProcess needs a special ordering, use these to prefix the values
	//! to ensure the deferred shading pass is after all LPV objects.
	enum EDeferredPreprocess
	{
		eDeferredShadingPass = BIT(30)
	};
	void   IncreaseOctreeCounter()   { nValue += eOctreeNodeCounter; }
	void   IncreaseObjectCounter()   { nValue += eObjectCounter; }
	void   IncreaseGroupCounter()    { nValue += eGroupCounter; }

	void   IncreaseParticleCounter() { nValue += eParticleCounter; }
	u32 ParticleCounter() const   { return nValue & ~eRecursivePassMask; }

	u32 GetValue() const          { return nValue; }

	bool   operator<(const SRendItemSorter& rOther) const
	{
		return nValue < rOther.nValue;
	}

	bool IsRecursivePass() const { return (nValue & eRecursivePassMask) != 0; }

	SRendItemSorter() : nValue(0) {}
	explicit SRendItemSorter(u32 _nValue) : nValue(_nValue) {}

private:
	//! Encode various counter in a single value.
	enum
	{
		eRecursivePassMask = BIT(31)  //!< Present in all combinations.
	};

	//! Flags used for regular SRendItems.
	enum
	{
		eObjectCounter = BIT(0)      //!< Bits 0-14 used.
	};
	enum
	{
		eOctreeNodeCounter = BIT(14) //!< Bits 15-27 used.
	};
	enum
	{
		eGroupCounter = BIT(27)      //!< Bits 28-31 used.
	};

	//! Flags used for Particles.
	enum { eParticleCounter = BIT(0) };

	u32 nValue;
};
//! \endcond

//! State of 3dengine during rendering.
//! Used to prevent global state.
struct SRenderingPassInfo
{
	operator SRenderObjectAccessThreadConfig() const
	{
		return SRenderObjectAccessThreadConfig(ThreadID());
	}

	enum EShadowMapType
	{
		SHADOW_MAP_NONE = 0,
		SHADOW_MAP_GSM,
		SHADOW_MAP_LOCAL,
		SHADOW_MAP_CACHED,
		SHADOW_MAP_CACHED_MGPU_COPY
	};

	//! Enum flags to identify which objects to skip for this pass.
	enum ESkipRenderingFlags
	{
		SHADOWS                    = BIT(0),
		BRUSHES                    = BIT(1),
		VEGETATION                 = BIT(2),
		ENTITIES                   = BIT(3),
		TERRAIN                    = BIT(4),
		WATEROCEAN                 = BIT(5),
		PARTICLES                  = BIT(6),
		DECALS                     = BIT(7),
		TERRAIN_DETAIL_MATERIALS   = BIT(8),
		FAR_SPRITES                = BIT(9),
		MERGED_MESHES              = BIT(10),
		WATER_WAVES                = BIT(12),
		ROADS                      = BIT(13),
		WATER_VOLUMES              = BIT(14),
		CLOUDS                     = BIT(15),
		CUBEMAP_GEN                = BIT(16),
		GEOM_CACHES                = BIT(17),
		DISABLE_RENDER_CHUNK_MERGE = BIT(18),

		// below are precombined flags
		STATIC_OBJECTS          = BRUSHES | VEGETATION,
		DEFAULT_FLAGS           = SHADOWS | BRUSHES | VEGETATION | ENTITIES | TERRAIN | WATEROCEAN | PARTICLES | DECALS | TERRAIN_DETAIL_MATERIALS | FAR_SPRITES | MERGED_MESHES | WATER_WAVES | ROADS | WATER_VOLUMES | CLOUDS | GEOM_CACHES,
		DEFAULT_SHADOWS_FLAGS   = BRUSHES | VEGETATION | ENTITIES | TERRAIN | WATEROCEAN | PARTICLES | DECALS | TERRAIN_DETAIL_MATERIALS | FAR_SPRITES | MERGED_MESHES | WATER_WAVES | ROADS | WATER_VOLUMES | CLOUDS | GEOM_CACHES,
		DEFAULT_RECURSIVE_FLAGS = BRUSHES | VEGETATION | ENTITIES | TERRAIN | WATEROCEAN | PARTICLES | DECALS | TERRAIN_DETAIL_MATERIALS | FAR_SPRITES | MERGED_MESHES | WATER_WAVES | ROADS | WATER_VOLUMES | CLOUDS | GEOM_CACHES
	};

	//! Creating function for RenderingPassInfo, the create functions will fetch all other necessary information like thread id/frame id, etc.
	static SRenderingPassInfo CreateGeneralPassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags = DEFAULT_FLAGS, bool bAuxWindow = false, SDisplayContextKey displayContextKey = {});
	static SRenderingPassInfo CreateRecursivePassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags = DEFAULT_RECURSIVE_FLAGS);
	static SRenderingPassInfo CreateShadowPassRenderingInfo(IRenderViewPtr pRenderView, const CCamera& rCamera, i32 nLightFlags, i32 nShadowMapLod, i32 nShadowCacheLod, bool bExtendedLod, bool bIsMGPUCopy, u32 nSide, u32 nRenderingFlags = DEFAULT_SHADOWS_FLAGS);
	static SRenderingPassInfo CreateBillBoardGenPassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags = DEFAULT_FLAGS);
	static SRenderingPassInfo CreateTempRenderingInfo(const CCamera& rCamera, const SRenderingPassInfo& rPassInfo);
	static SRenderingPassInfo CreateTempRenderingInfo(u32 nRenderingFlags, const SRenderingPassInfo& rPassInfo);
	static SRenderingPassInfo CreateTempRenderingInfo(SRendItemSorter s, const SRenderingPassInfo& rPassInfo);

	// state getter
	bool                    IsGeneralPass() const;

	bool                    IsRecursivePass() const;
	u32                  GetRecursiveLevel() const;

	bool                    IsShadowPass() const;
	bool                    IsCachedShadowPass() const;
	EShadowMapType          GetShadowMapType() const;
	bool                    IsDisableRenderChunkMerge() const;

	bool                    IsAuxWindow() const;

	threadID                ThreadID() const;
	void                    SetThreadID(threadID id) { m_nThreadID = static_cast<u8>(id); }

	i32                     GetFrameID() const;
	u32                  GetMainFrameID() const;

	const CCamera&          GetCamera() const;
	bool                    IsCameraUnderWater() const;

	float                   GetZoomFactor() const;
	float                   GetInverseZoomFactor() const;
	bool                    IsZoomActive() const;
	bool                    IsZoomInProgress() const;

	bool                    RenderShadows() const;
	bool                    RenderBrushes() const;
	bool                    RenderVegetation() const;
	bool                    RenderEntities() const;
	bool                    RenderTerrain() const;
	bool                    RenderWaterOcean() const;
	bool                    RenderParticles() const;
	bool                    RenderDecals() const;
	bool                    RenderTerrainDetailMaterial() const;
	bool                    RenderFarSprites() const;
	bool                    RenderMergedMeshes() const;
	bool                    RenderWaterWaves() const;
	bool                    RenderRoads() const;
	bool                    RenderWaterVolumes() const;
	bool                    RenderClouds() const;
	bool                    RenderGeomCaches() const;

	bool                    IsRenderingCubemap() const;

	u8                   ShadowFrustumSide() const;
	u8                   ShadowFrustumLod() const;
	u8                   ShadowCacheLod() const;

	CRenderView*            GetRenderView() const;
	IRenderView*            GetIRenderView() const;

	SRendItemSorter&        GetRendItemSorter() const                   { return m_renderItemSorter; };
	void                    OverrideRenderItemSorter(SRendItemSorter s) { m_renderItemSorter = s; }

	const SDisplayContextKey& GetDisplayContextKey() const   { return m_displayContextKey; }

	void                             SetShadowPasses(class std::vector<SRenderingPassInfo>* p) { m_pShadowPasses = p; }
	std::vector<SRenderingPassInfo>* GetShadowPasses() const                                   { return m_pShadowPasses; }

	SRenderingPassInfo(threadID id)
	{
		SetThreadID(id);
	}

private:
	//! Private constructor, creation is only allowed with create functions.
	SRenderingPassInfo()
	{
		threadID nThreadID = 0;
		gEnv->pRenderer->EF_Query(EFQ_MainThreadList, nThreadID);
		m_nThreadID = static_cast<u8>(nThreadID);
		m_nRenderMainFrameID = gEnv->nMainFrameID;
	}

	void InitRenderingFlags(u32 nRenderingFlags);
	void SetCamera(const CCamera& cam);

	void SetRenderView(i32 nThreadID, IRenderView::EViewType Type = IRenderView::eViewType_Default);
	void SetRenderView(IRenderViewPtr pRenderView);
	void SetRenderView(IRenderView* pRenderView);

	u8  m_nThreadID = 0;
	u8  m_nRenderStackLevel = 0;
	u8  m_eShadowMapRendering = static_cast<u8>(SHADOW_MAP_NONE);   //!< State flag denoting what type of shadow map is being currently rendered into.
	u8  m_bCameraUnderWater = false;

	u32 m_nRenderingFlags = 0;

	float  m_fZoomFactor = 0.0f;

	u32 m_nRenderMainFrameID = 0;

	// Current pass render item sorter.
	mutable SRendItemSorter m_renderItemSorter;

	const CCamera*          m_pCamera = nullptr;

	// Render view used for this rendering pass
	IRenderViewPtr m_pRenderView;

	// members used only in shadow pass
	u8   nShadowSide;
	u8   nShadowLod;
	u8   nShadowCacheLod = 0;
	u8   m_nZoomInProgress = false;
	u8   m_nZoomMode = 0;
	u8   m_bAuxWindow = false;

	// Windows handle of the target Display Context in the multi-context rendering (in Editor)
	SDisplayContextKey m_displayContextKey;

	// Optional render target clear color.
	ColorB m_clearColor = { 0, 0, 0, 0 };

	// Additional sub-passes like shadow frustums (in the future - reflections and portals)
	std::vector<SRenderingPassInfo>* m_pShadowPasses = nullptr;
};

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsGeneralPass() const
{
	return m_nRenderStackLevel == 0 && m_bAuxWindow == 0 && static_cast<EShadowMapType>(m_eShadowMapRendering) == SHADOW_MAP_NONE;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsRecursivePass() const
{
	return m_nRenderStackLevel > 0;
}

///////////////////////////////////////////////////////////////////////////////
inline u32 SRenderingPassInfo::GetRecursiveLevel() const
{
	return m_nRenderStackLevel;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsShadowPass() const
{
	return static_cast<EShadowMapType>(m_eShadowMapRendering) != SHADOW_MAP_NONE;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsCachedShadowPass() const
{
	return IsShadowPass() &&
	       (GetShadowMapType() == SRenderingPassInfo::SHADOW_MAP_CACHED ||
	        GetShadowMapType() == SRenderingPassInfo::SHADOW_MAP_CACHED_MGPU_COPY);
}
///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo::EShadowMapType SRenderingPassInfo::GetShadowMapType() const
{
	assert(IsShadowPass());
	return static_cast<EShadowMapType>(m_eShadowMapRendering);
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsAuxWindow() const
{
	return m_bAuxWindow != 0;
}

///////////////////////////////////////////////////////////////////////////////
inline threadID SRenderingPassInfo::ThreadID() const
{
	return m_nThreadID;
}

///////////////////////////////////////////////////////////////////////////////
inline i32 SRenderingPassInfo::GetFrameID() const
{
	return (i32)m_nRenderMainFrameID;
}

///////////////////////////////////////////////////////////////////////////////
inline u32 SRenderingPassInfo::GetMainFrameID() const
{
	return m_nRenderMainFrameID;
}

///////////////////////////////////////////////////////////////////////////////
inline const CCamera& SRenderingPassInfo::GetCamera() const
{
	assert(m_pCamera != NULL);
	return *m_pCamera;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsCameraUnderWater() const
{
	return m_bCameraUnderWater != 0;
}

///////////////////////////////////////////////////////////////////////////////
inline float SRenderingPassInfo::GetZoomFactor() const
{
	return m_fZoomFactor;
}

///////////////////////////////////////////////////////////////////////////////
inline float SRenderingPassInfo::GetInverseZoomFactor() const
{
	return 1.0f / m_fZoomFactor;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsZoomActive() const
{
	return m_nZoomMode != 0;
}

///////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsZoomInProgress() const
{
	return m_nZoomInProgress != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderShadows() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::SHADOWS) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderBrushes() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::BRUSHES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderVegetation() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::VEGETATION) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderEntities() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::ENTITIES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderTerrain() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::TERRAIN) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderWaterOcean() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::WATEROCEAN) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderParticles() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::PARTICLES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderDecals() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::DECALS) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderFarSprites() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::FAR_SPRITES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderMergedMeshes() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::MERGED_MESHES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderTerrainDetailMaterial() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::TERRAIN_DETAIL_MATERIALS) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderWaterWaves() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::WATER_WAVES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderRoads() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::ROADS) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderWaterVolumes() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::WATER_VOLUMES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderClouds() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::CLOUDS) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::RenderGeomCaches() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::GEOM_CACHES) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsRenderingCubemap() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::CUBEMAP_GEN) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline bool SRenderingPassInfo::IsDisableRenderChunkMerge() const
{
	return (m_nRenderingFlags& SRenderingPassInfo::DISABLE_RENDER_CHUNK_MERGE) != 0;
}

////////////////////////////////////////////////////////////////////////////////
inline u8 SRenderingPassInfo::ShadowFrustumSide() const
{
	return nShadowSide;
}

////////////////////////////////////////////////////////////////////////////////
inline u8 SRenderingPassInfo::ShadowFrustumLod() const
{
	return nShadowLod;
}

////////////////////////////////////////////////////////////////////////////////
inline u8 SRenderingPassInfo::ShadowCacheLod() const
{
	return nShadowCacheLod;
}

////////////////////////////////////////////////////////////////////////////////
inline CRenderView* SRenderingPassInfo::GetRenderView() const
{
	return reinterpret_cast<CRenderView*>(m_pRenderView.get());
}

////////////////////////////////////////////////////////////////////////////////
inline IRenderView* SRenderingPassInfo::GetIRenderView() const
{
	return m_pRenderView.get();
}

////////////////////////////////////////////////////////////////////////////////
inline void SRenderingPassInfo::SetCamera(const CCamera& cam)
{
	cam.CalculateRenderMatrices();
	m_pCamera = gEnv->p3DEngine->GetRenderingPassCamera(cam);
	m_bCameraUnderWater = gEnv->p3DEngine->IsUnderWater(cam.GetPosition());
	m_fZoomFactor = 0.4f + 0.6f * (RAD2DEG(cam.GetFov()) / 60.f);
	m_nZoomInProgress = 0;
	m_nZoomMode = 0;
}

////////////////////////////////////////////////////////////////////////////////
inline void SRenderingPassInfo::InitRenderingFlags(u32 nRenderingFlags)
{
	m_nRenderingFlags = nRenderingFlags;
#if !defined(_RELEASE)
	static ICVar* pDefaultMaterial = gEnv->pConsole->GetCVar("e_DefaultMaterial");
	static ICVar* pDetailMaterial = gEnv->pConsole->GetCVar("e_TerrainDetailMaterials");
	static ICVar* pShadows = gEnv->pConsole->GetCVar("e_Shadows");
	static ICVar* pBrushes = gEnv->pConsole->GetCVar("e_Brushes");
	static ICVar* pVegetation = gEnv->pConsole->GetCVar("e_Vegetation");
	static ICVar* pEntities = gEnv->pConsole->GetCVar("e_Entities");
	static ICVar* pTerrain = gEnv->pConsole->GetCVar("e_Terrain");
	static ICVar* pWaterOcean = gEnv->pConsole->GetCVar("e_WaterOcean");
	static ICVar* pParticles = gEnv->pConsole->GetCVar("e_Particles");
	static ICVar* pDecals = gEnv->pConsole->GetCVar("e_Decals");
	static ICVar* pWaterWaves = gEnv->pConsole->GetCVar("e_WaterWaves");
	static ICVar* pWaterVolumes = gEnv->pConsole->GetCVar("e_WaterVolumes");
	static ICVar* pRoads = gEnv->pConsole->GetCVar("e_Roads");
	static ICVar* pClouds = gEnv->pConsole->GetCVar("e_Clouds");
	static ICVar* pGeomCaches = gEnv->pConsole->GetCVar("e_GeomCaches");
	static ICVar* pMergedMeshes = gEnv->pConsole->GetCVar("e_MergedMeshes");

	if (pShadows->GetIVal() == 0)        m_nRenderingFlags &= ~SRenderingPassInfo::SHADOWS;
	if (pBrushes->GetIVal() == 0)        m_nRenderingFlags &= ~SRenderingPassInfo::BRUSHES;
	if (pVegetation->GetIVal() == 0)     m_nRenderingFlags &= ~SRenderingPassInfo::VEGETATION;
	if (pEntities->GetIVal() == 0)       m_nRenderingFlags &= ~SRenderingPassInfo::ENTITIES;
	if (pTerrain->GetIVal() == 0)        m_nRenderingFlags &= ~SRenderingPassInfo::TERRAIN;
	if (pWaterOcean->GetIVal() == 0)     m_nRenderingFlags &= ~SRenderingPassInfo::WATEROCEAN;
	if (pParticles->GetIVal() == 0)      m_nRenderingFlags &= ~SRenderingPassInfo::PARTICLES;
	if (pDecals->GetIVal() == 0)         m_nRenderingFlags &= ~SRenderingPassInfo::DECALS;
	if (pWaterWaves->GetIVal() == 0)     m_nRenderingFlags &= ~SRenderingPassInfo::WATER_WAVES;
	if (pRoads->GetIVal() == 0)          m_nRenderingFlags &= ~SRenderingPassInfo::ROADS;
	if (pWaterVolumes->GetIVal() == 0)   m_nRenderingFlags &= ~SRenderingPassInfo::WATER_VOLUMES;
	if (pClouds->GetIVal() == 0)         m_nRenderingFlags &= ~SRenderingPassInfo::CLOUDS;
	if (pGeomCaches->GetIVal() == 0)     m_nRenderingFlags &= ~SRenderingPassInfo::GEOM_CACHES;
	if (pMergedMeshes->GetIVal() == 0)   m_nRenderingFlags &= ~SRenderingPassInfo::MERGED_MESHES;

	if (pDefaultMaterial->GetIVal() != 0 || pDetailMaterial->GetIVal() == 0)
		m_nRenderingFlags &= ~SRenderingPassInfo::TERRAIN_DETAIL_MATERIALS;

	// on dedicated server, never render any object at all
	if (gEnv->IsDedicated())
		m_nRenderingFlags = 0;
#endif
}

//////////////////////////////////////////////////////////////////////////

inline void SRenderingPassInfo::SetRenderView(i32 nThreadID, IRenderView::EViewType Type)
{
	m_pRenderView = reinterpret_cast<IRenderView*>(gEnv->pRenderer->GetOrCreateRenderView(Type));
	SetRenderView(m_pRenderView.get());
}

inline void SRenderingPassInfo::SetRenderView(IRenderViewPtr pRenderView)
{
	SetRenderView(pRenderView.get());
	m_pRenderView = std::move(pRenderView);
}

inline void SRenderingPassInfo::SetRenderView(IRenderView* pRenderView)
{
	pRenderView->SetSkipRenderingFlags(m_nRenderingFlags);
	pRenderView->SetFrameId(GetFrameID());
	pRenderView->SetFrameTime(gEnv->pTimer->GetFrameStartTime(ITimer::ETIMER_UI));
	pRenderView->SetViewport(SRenderViewport(0, 0, m_pCamera->GetViewSurfaceX(), m_pCamera->GetViewSurfaceZ()));
}

////////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateBillBoardGenPassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags)
{
	const CCamera& rCameraToSet = rCamera;

	SRenderingPassInfo passInfo;

	passInfo.SetCamera(rCameraToSet);
	passInfo.InitRenderingFlags(nRenderingFlags);
	passInfo.SetRenderView(passInfo.ThreadID(), IRenderView::eViewType_BillboardGen);

	passInfo.m_bAuxWindow = false;
	passInfo.m_displayContextKey = {};
	passInfo.m_renderItemSorter.nValue = 0;

	return passInfo;
}

////////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateGeneralPassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags, bool bAuxWindow, SDisplayContextKey displayContextKey)
{
	static ICVar* pCameraFreeze = gEnv->pConsole->GetCVar("e_CameraFreeze");

	// Update Camera only if e_camerafreeze is not set
	const CCamera& rCameraToSet = (pCameraFreeze && pCameraFreeze->GetIVal() != 0) ? gEnv->p3DEngine->GetRenderingCamera() : rCamera;

	SRenderingPassInfo passInfo;

	passInfo.SetCamera(rCameraToSet);
	passInfo.InitRenderingFlags(nRenderingFlags);
	passInfo.SetRenderView(passInfo.ThreadID(), IRenderView::eViewType_Default);

	passInfo.m_bAuxWindow = bAuxWindow;
	passInfo.m_displayContextKey = displayContextKey;
	passInfo.m_renderItemSorter.nValue = 0;

	// update general pass zoom factor
	float fPrevZoomFactor = gEnv->p3DEngine->GetPrevZoomFactor();
	passInfo.m_nZoomMode = gEnv->p3DEngine->GetZoomMode();
	passInfo.m_nZoomInProgress = passInfo.m_nZoomMode && fabs(fPrevZoomFactor - passInfo.m_fZoomFactor) > 0.02f;

	i32 nZoomMode = passInfo.m_nZoomMode;
	const float fZoomThreshold = 0.7f;
	if (passInfo.m_fZoomFactor < fZoomThreshold)
		++nZoomMode;
	else
		--nZoomMode;

	// clamp zoom mode into valid range
	passInfo.m_nZoomMode = clamp_tpl<i32>(nZoomMode, 0, 4);

	// store information for next frame in 3DEngine
	gEnv->p3DEngine->SetPrevZoomFactor(passInfo.m_fZoomFactor);
	gEnv->p3DEngine->SetZoomMode(passInfo.m_nZoomMode);

	return passInfo;
}

///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateRecursivePassRenderingInfo(const CCamera& rCamera, u32 nRenderingFlags)
{
	static ICVar* pRecursionViewDistRatio = gEnv->pConsole->GetCVar("e_RecursionViewDistRatio");

	SRenderingPassInfo passInfo;

	passInfo.SetCamera(rCamera);
	passInfo.InitRenderingFlags(nRenderingFlags);
	passInfo.SetRenderView(passInfo.ThreadID(), IRenderView::eViewType_Recursive);

	//	passInfo.m_bAuxWindow = bAuxWindow;
	passInfo.m_renderItemSorter.nValue = SRendItemSorter::eRecursivePassMask;
	passInfo.m_nRenderStackLevel = 1;

	// adjust view distance in recursive mode by adjusting the ZoomFactor
	passInfo.m_fZoomFactor /= pRecursionViewDistRatio->GetFVal();

	return passInfo;
}

///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateShadowPassRenderingInfo(IRenderViewPtr pRenderView, const CCamera& rCamera, i32 nLightFlags, i32 nShadowMapLod, i32 nShadowCacheLod, bool bExtendedLod, bool bIsMGPUCopy, u32 nSide, u32 nRenderingFlags)
{
	SRenderingPassInfo passInfo;

	passInfo.SetCamera(rCamera);
	passInfo.InitRenderingFlags(nRenderingFlags);
	passInfo.SetRenderView(pRenderView);

	// set correct shadow map type
	if (nLightFlags & DLF_SUN)
	{
		assert(nShadowMapLod >= 0 && nShadowMapLod < 8);
		if (bExtendedLod)
			passInfo.m_eShadowMapRendering = bIsMGPUCopy ? SHADOW_MAP_CACHED_MGPU_COPY : SHADOW_MAP_CACHED;
		else
			passInfo.m_eShadowMapRendering = SHADOW_MAP_GSM;
	}
	else if (nLightFlags & (DLF_POINT | DLF_PROJECT | DLF_AREA_LIGHT))
		passInfo.m_eShadowMapRendering = static_cast<u8>(SHADOW_MAP_LOCAL);
	else
		passInfo.m_eShadowMapRendering = static_cast<u8>(SHADOW_MAP_NONE);

	passInfo.nShadowSide = nSide;
	passInfo.nShadowLod = nShadowMapLod;
	passInfo.nShadowCacheLod = nShadowCacheLod;
	return passInfo;
}

///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateTempRenderingInfo(const CCamera& rCamera, const SRenderingPassInfo& rPassInfo)
{
	SRenderingPassInfo passInfo = rPassInfo;

	passInfo.SetCamera(rCamera);
	passInfo.nShadowSide = 0;

	return passInfo;
}

///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateTempRenderingInfo(u32 nRenderingFlags, const SRenderingPassInfo& rPassInfo)
{
	SRenderingPassInfo passInfo = rPassInfo;

	passInfo.SetRenderView(nullptr);
	passInfo.m_nRenderingFlags = nRenderingFlags;
	passInfo.GetIRenderView()->SetSkipRenderingFlags(nRenderingFlags);

	return passInfo;
}

///////////////////////////////////////////////////////////////////////////////
inline SRenderingPassInfo SRenderingPassInfo::CreateTempRenderingInfo(SRendItemSorter s, const SRenderingPassInfo& rPassInfo)
{
	SRenderingPassInfo passInfo = rPassInfo;

	passInfo.OverrideRenderItemSorter(s);

	return passInfo;
}
