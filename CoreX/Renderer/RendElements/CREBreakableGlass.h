// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

// Debugging (This should *always* be committed disabled...)
#if !defined(RELEASE)
//#define GLASS_DEBUG_MODE
#endif

// Includes
#include "CREBreakableGlassHelpers.h"

// Forward decls
struct IRenderAuxGeom;
struct SAuxGeomRenderFlags;
struct SPipTangents;
class CDrxNameR;

template<class T, uint N>
class FixedPodArray;

#if GLASSCFG_USE_HASH_GRID
template<class T, u32 GridSize, u32 BucketSize>
class CSpatialHashGrid;
#endif

// Typedefs
#if GLASSCFG_USE_HASH_GRID
typedef CSpatialHashGrid<u8, GLASSCFG_HASH_GRID_SIZE, GLASSCFG_HASH_GRID_BUCKET_SIZE> TGlassHashGrid;
typedef DrxFixedArray<u8, GLASSCFG_HASH_GRID_BUCKET_SIZE>                             TGlassHashBucket;
#endif

typedef FixedPodArray<i32, GLASSCFG_NUM_RADIAL_CRACKS>              TRadialCrackArray;
typedef DrxFixedArray<SGlassFragment, GLASSCFG_FRAGMENT_ARRAY_SIZE> TFragArray;
typedef DrxFixedArray<u8, GLASSCFG_FRAGMENT_ARRAY_SIZE>          TFragIndexArray;
typedef FixedPodArray<u8, GLASSCFG_FRAGMENT_ARRAY_SIZE>          TFragIndexPodArray;
typedef FixedPodArray<u8, GLASSCFG_NUM_RADIAL_CRACKS>            TSubFragArray;
typedef DrxFixedArray<SGlassImpactParams, GLASSCFG_MAX_NUM_IMPACTS> TImpactArray;

//! Breakable glass sim render element params.
struct SBreakableGlassREParams
{
	SBreakableGlassREParams()
		: centre(Vec3Constants<f32>::fVec3_Zero)
	{
		matrix.SetIdentity();
	}

	Vec3     centre;
	Matrix34 matrix;
};

//! Breakable glass sim render element.
class CREBreakableGlass : public CRenderElement
{
public:
	CREBreakableGlass();
	virtual ~CREBreakableGlass();

	// CREBreakableGlass interface
	virtual bool InitialiseRenderElement(const SBreakableGlassInitParams& params);
	virtual void ReleaseRenderElement();

	virtual void Update(SBreakableGlassUpdateParams& params);
	virtual void GetGlassState(SBreakableGlassState& state);

	virtual void ApplyImpactToGlass(const SGlassImpactParams& params);
	virtual void ApplyExplosionToGlass(const SGlassImpactParams& params);

	virtual void SetCVars(const SBreakableGlassCVars* pCVars);

#ifdef GLASS_DEBUG_MODE
	virtual void DrawFragmentDebug(const uint fragIndex, const Matrix34& matrix, u8k buffId, const float alpha);
#endif

	ILINE SBreakableGlassREParams* GetParams()
	{
		return &m_params;
	}

private:
	// Internal constants
	static CDrxNameR s_ImpactDecalParamName;

	enum EGlassSurfaceSide
	{
		EGlassSurfaceSide_Center,
		EGlassSurfaceSide_Front,
		EGlassSurfaceSide_Back
	};

	// Core
	void ResetTempCrackData();
	void RT_UpdateBuffers(i32k subFragIndex = -1);

	// Random
	void  SeedRand();
	float GetRandF();
	float GetRandF(const uint index);

	// Glass impact state/reaction
	void  HandleAdditionalImpact(const SGlassImpactParams& params);
	void  ShatterGlass();
	void  PlayShatterEffect(u8k fragIndex);

	float CalculateBreakThreshold();
	bool  CheckForGlassBreak(const float breakThreshold, float& excessEnergy);
	void  CalculateImpactEnergies(const float totalEnergy, float& radialEnergy, float& circularEnergy);

	// Crack pattern generation
	uint  GenerateCracksFromImpact(u8k parentFragIndex, const SGlassImpactParams& impact);
	float PropagateRadialCrack(const uint startIndex, const uint currIndex, const uint localIndex, const float angle, const float energyPerStep);
	uint  GenerateRadialCracks(const float totalEnergy, const uint impactDepth, const float fragArea);
	float GetImpactRadius(const SGlassImpactParams& impact);
	float GetDecalRadius(u8k impactIndex);

	// Fragment impacts
	bool            ApplyImpactToFragments(SGlassImpactParams& impact);
	bool            FindImpactFragment(const SGlassImpactParams& impact, u8& fragIndex);

	SGlassFragment* AddFragment();
	void            RemoveFragment(u8k fragIndex);
	void            RebuildChangedFragment(u8k fragIndex);

	void            GenerateSubFragments(u8k parentFragIndex, const SGlassImpactParams& impact);
	void            GenerateSubFragmentCracks(u8k parentFragIndex, const SGlassImpactParams& impact, TRadialCrackArray& fragIntersectPts);
	bool            CreateFragmentFromOutline(const Vec2* pOutline, i32k outlineSize, u8k parentFragIndex, const bool forceLoose = false);

	bool            IsFragmentLoose(const SGlassFragment& frag);
	bool            IsFragmentWeaklyLinked(const SGlassFragment& frag);
	void            RemoveFragmentConnections(u8k fragIndex, PodArray<u8>& connections);

	void            ConnectSubFragments(u8k* pSubFrags, u8k numSubFrags);
	void            ReplaceFragmentConnections(u8k parentFragIndex, u8k* pSubFrags, u8k numSubFrags);
	void            ConnectFragmentPair(u8k fragAIndex, u8k fragBIndex);

	void            FindLooseFragments(TGlassPhysFragmentArray* pPhysFrags, TGlassPhysFragmentInitArray* pPhysFragsInitData);
	void            TraverseStableConnection(u8k fragIndex, TFragIndexPodArray& stableFrags);
	void            PrePhysicalizeLooseFragment(TGlassPhysFragmentArray* pPhysFrags, TGlassPhysFragmentInitArray* pPhysFragsInitData, u8k fragIndex, const bool dampenVel, const bool noVel);

	// Fragment hashing
	void SetHashGridSize(const Vec2& size);
	void HashFragment(u8k fragIndex, const bool remove = false);

	void HashFragmentTriangle(const Vec2& a, const Vec2& b, const Vec2& c,
	                          u8k& triFrag, const bool removeTri = false);

	void HashFragmentTriangleSpan(const struct STriEdge& a, const struct STriEdge& b,
	                              u8k& triFrag, const bool removeTri = false);

	// Fragment management
	void DeactivateFragment(u8k index, u32k bit);

	// Geometry generation
	void  GenerateLoosePieces(i32k numRadialCracks);
	void  GenerateStablePieces(i32k numRadialCracks);
	float GetClosestImpactDistance(const Vec2& pt);

	void  RebuildAllFragmentGeom();
	void  RehashAllFragmentData();
	void  ProcessFragmentGeomData();
	void  BuildFragmentTriangleData(u8k fragIndex, uint& pVertOffs, uint& pIndOffs, const EGlassSurfaceSide side = EGlassSurfaceSide_Center, i32k subFragID = -1);
	void  BuildFragmentOutlineData(u8k fragIndex, uint& pVertOffs, uint& pIndOffs, i32k subFragID = -1);

	void  GenerateDefaultPlaneGeom();
	bool  GenerateGeomFromFragment(const Vec2* pFragPts, const uint numPts);
	void  GenerateVertFromPoint(const Vec3& pt, const Vec2& uvOffset, SVF_P3F_C4B_T2F& vert, const bool impactDistInAlpha = false);
	void  GenerateTriangleTangent(const Vec3& triPt0, const Vec3& triPt1, const Vec3& triPt2, SPipTangents& tangent);
	void  PackTriangleTangent(const Vec3& tangent, const Vec3& bitangent, SPipTangents& tanBitan);

	// Drawing
	void UpdateImpactShaderConstants();
	void SetImpactShaderConstants(CShader* pShader);

#ifdef GLASS_DEBUG_MODE
	// Debug geometry
	void GenerateImpactGeom(const SGlassImpactParams& impact);
	void TransformPointList(PodArray<Vec3>& ptList, const bool inverse = false);

	// Debug drawing
	void DebugDraw(const bool wireframe, const bool data);
	void DrawLooseGeom(IRenderAuxGeom* const pRenderer, SAuxGeomRenderFlags& flags);
	void DrawOutlineGeom(IRenderAuxGeom* const pRenderer, SAuxGeomRenderFlags& flags);
	void DrawFragmentDebug(IRenderAuxGeom* const pRenderer, SAuxGeomRenderFlags& flags);
	void DrawFragmentConnectionDebug(IRenderAuxGeom* const pRenderer, SAuxGeomRenderFlags& flags);
	void DrawDebugData(IRenderAuxGeom* const pRenderer, SAuxGeomRenderFlags& flags);
#endif

	//! Geometry buffer
	struct SGlassGeom
	{
		void Clear()
		{
			m_verts.Clear();
			m_inds.Clear();
			m_tans.Clear();
		}

		void Free()
		{
			m_verts.Free();
			m_inds.Free();
			m_tans.Free();
		}

		PodArray<SVF_P3F_C4B_T2F> m_verts;
		PodArray<u16>          m_inds;
		PodArray<SPipTangents>    m_tans;
	};

	//! Physicalized fragment
	struct SGlassPhysFragId
	{
		u32 m_geomBufferId;
		u8  m_fragId;
	};

	// Shared data
	static float s_loosenTimer;
	static float s_impactTimer;

	// Persistent data
	TFragArray                   m_frags;
	TImpactArray                 m_impactParams;
	TFragIndexArray              m_freeFragIndices;

	SBreakableGlassInitParams    m_glassParams;
	SBreakableGlassREParams      m_params;
	SBreakableGlassDecalConstant m_decalPSConsts[GLASSCFG_MAX_NUM_IMPACT_DECALS];

	// Temp crack generation data
	Vec2            m_pointArray[GLASSCFG_MAX_NUM_CRACK_POINTS];
	PodArray<Vec2*> m_radialCrackTrees[GLASSCFG_NUM_RADIAL_CRACKS];

	// Temp fragment mesh data
	SGlassGeom       m_fragFullGeom[GLASSCFG_MAX_NUM_PHYS_FRAGMENTS];
	SGlassPhysFragId m_fragGeomBufferIds[GLASSCFG_MAX_NUM_PHYS_FRAGMENTS];
	 bool    m_fragGeomRebuilt[GLASSCFG_MAX_NUM_PHYS_FRAGMENTS];

	// Temp stable mesh data
	SGlassGeom m_planeGeom;
	SGlassGeom m_crackGeom;

#ifdef GLASS_DEBUG_MODE
	// Debug drawing
	PodArray<Vec3> m_impactLineList;
#endif

	// Persistent data
	static const SBreakableGlassCVars* s_pCVars;

	Vec2                               m_hashCellSize;
	Vec2                               m_hashCellInvSize;
	Vec2                               m_hashCellHalfSize;
	Vec2                               m_invUVRange;

#if GLASSCFG_USE_HASH_GRID
	TGlassHashGrid* m_pHashGrid;
#endif
	float           m_invMaxGlassSize;
	float           m_impactEnergy;
	u32          m_geomBufferId;
	u32          m_seed;

	// Fragment state bit arrays
	u32 m_fragsActive;
	u32 m_fragsLoose;
	u32 m_fragsFree;
	u32 m_lastFragBit;

	// Shader constant state
	u8 m_numDecalImpacts;
	u8 m_numDecalPSConsts;

	// Temp counts
	u8 m_pointArrayCount;
	u8 m_lastPhysFrag;
	u8 m_totalNumFrags;
	u8 m_lastFragIndex;
	u8 m_numLooseFrags;

	// Glass state
	bool            m_shattered;
	bool            m_geomDirty;
	 bool   m_geomRebuilt;                              //!< Accessed by MT and RT
	 bool   m_geomBufferLost;                           //!< Accessed by MT and RT
	 u8  m_dirtyGeomBufferCount;                     //!< Accessed by MT and RT
	 u32 m_geomUpdateFrame;
};

//! \endcond