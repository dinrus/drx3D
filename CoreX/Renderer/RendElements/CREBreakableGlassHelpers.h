// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include "CREBreakableGlassConfig.h"
#include <drx3D/CoreX/Containers/DrxFixedArray.h>

// Forward declarations
struct EventPhysCollision;
struct IMaterialEffects;
struct IParticleEffect;
struct IPhysicalEntity;
struct phys_geometry;
struct IRenderNode;
struct IStatObj;

//! Breakable glass sim initialisation params.
struct SBreakableGlassInitParams
{
	SBreakableGlassInitParams()
		: uvOrigin(Vec2Constants<f32>::fVec2_Zero)
		, uvXAxis(Vec2Constants<f32>::fVec2_OneX)
		, uvYAxis(Vec2Constants<f32>::fVec2_OneY)
		, size(Vec2Constants<f32>::fVec2_One)
		, thickness(0.01f)
		, pGlassMaterial(NULL)
		, pShatterEffect(NULL)
		, surfaceTypeId(0)
		, pInitialFrag(NULL)
		, numInitialFragPts(0)
		, numAnchors(0)
	{
		memset(anchors, 0, sizeof(anchors));
	}

	static const uint MaxNumAnchors = 4;
	Vec2              anchors[MaxNumAnchors];

	Vec2              uvOrigin;
	Vec2              uvXAxis;
	Vec2              uvYAxis;

	Vec2              size;
	float             thickness;

	IMaterial*        pGlassMaterial;
	IParticleEffect*  pShatterEffect;
	i32               surfaceTypeId;

	Vec2*             pInitialFrag;
	u8             numInitialFragPts;
	u8             numAnchors;
};

//! Glass impact parameters (controlling split generation).
struct SGlassImpactParams
{
	SGlassImpactParams()
		: velocity(Vec3Constants<f32>::fVec3_Zero)
		, x(0.0f)
		, y(0.0f)
		, impulse(0.0f)
		, speed(0.0f)
		, radius(0.0f)
		, seed(0)
	{
	}

	Vec3   velocity;
	float  x;
	float  y;
	float  impulse;
	float  speed;
	float  radius;
	u32 seed;
};

//! Glass mesh fragment/polygon.
struct SGlassFragment
{
	SGlassFragment()
	{
		Reset();
	}

	void Reset()
	{
		m_center = Vec2Constants<float>::fVec2_Zero;
		m_area = 0.0f;
		m_depth = 0;

		m_outlinePts.Free();
		m_triInds.Free();
		m_outConnections.Free();
		m_inConnections.Free();
	}

	PodArray<Vec2>  m_outlinePts;
	PodArray<u8> m_triInds;
	PodArray<u8> m_outConnections;
	PodArray<u8> m_inConnections;

	Vec2            m_center;
	float           m_area;
	u8           m_depth;
};

//! Glass physicalized fragment initialisation data.
struct SGlassPhysFragmentInitData
{
	SGlassPhysFragmentInitData()
		: m_impulse(Vec3Constants<f32>::fVec3_Zero)
		, m_impulsePt(Vec3Constants<f32>::fVec3_Zero)
		, m_center(Vec2Constants<f32>::fVec2_Zero)
	{
	}

	Vec3 m_impulse;
	Vec3 m_impulsePt;
	Vec2 m_center;
};

typedef DrxFixedArray<SGlassPhysFragmentInitData, GLASSCFG_MAX_NUM_PHYS_FRAGMENTS> TGlassPhysFragmentInitArray;

//! Glass physicalized fragment.
struct SGlassPhysFragment
{
	SGlassPhysFragment()
		: m_size(0.0f)
		, m_pPhysEnt(NULL)
		, m_pRenderNode(NULL)
		, m_lifetime(0.0f)
		, m_fragIndex(GLASSCFG_FRAGMENT_ARRAY_SIZE)       //!< Array bounds, so invalid value
		, m_bufferIndex(GLASSCFG_MAX_NUM_PHYS_FRAGMENTS)  //!< Array bounds, so invalid value
		, m_initialised(false)
	{
		m_matrix.SetIdentity();
	}

	Matrix34         m_matrix;
	float            m_size;
	IPhysicalEntity* m_pPhysEnt;
	IRenderNode*     m_pRenderNode;
	float            m_lifetime;
	u8            m_fragIndex;
	u8            m_bufferIndex;
	bool             m_initialised;
};

typedef DrxFixedArray<SGlassPhysFragment, GLASSCFG_MAX_NUM_PHYS_FRAGMENTS> TGlassPhysFragmentArray;
typedef DrxFixedArray<u16, GLASSCFG_MAX_NUM_PHYS_FRAGMENTS>             TGlassPhysFragmentIdArray;

//! Glass break state
struct SBreakableGlassState
{
	SBreakableGlassState()
		: m_numImpacts(0)
		, m_numLooseFrags(0)
		, m_hasShattered(false)
	{
	}

	u8 m_numImpacts;
	u8 m_numLooseFrags;
	bool  m_hasShattered;
};

//! Glass update parameters passing data in and out.
struct SBreakableGlassUpdateParams
{
	SBreakableGlassUpdateParams()
		: m_frametime(0.0f)
		, m_pPhysFrags(NULL)
		, m_pPhysFragsInitData(NULL)
		, m_geomChanged(false)
	{
	}

	float                        m_frametime;
	TGlassPhysFragmentArray*     m_pPhysFrags;
	TGlassPhysFragmentInitArray* m_pPhysFragsInitData;
	TGlassPhysFragmentIdArray*   m_pDeadPhysFrags;
	bool                         m_geomChanged;
};

// Breakable glass physical data used during mesh extraction.

// Using a large size here, but most are just quads
// Note: Should keep size in sync with PolygonMath2D::POLY_ARRAY_SIZE
typedef DrxFixedArray<Vec2, 45> TGlassDefFragmentArray;
typedef DrxFixedArray<i32, 45>  TGlassFragmentIndexArray;

struct SBreakableGlassPhysData
{
	SBreakableGlassPhysData()
		: pStatObj(NULL)
		, pPhysGeom(NULL)
		, renderFlags(0)
	{
		entityMat.SetIdentity();

		// Default to a simple tiling pattern
		uvBasis[0] = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
		uvBasis[1] = Vec4(1.0f, 0.0f, 1.0f, 0.0f);
		uvBasis[2] = Vec4(0.0f, 1.0f, 0.0f, 1.0f);
	}

	TGlassDefFragmentArray defaultFrag;

	Matrix34               entityMat;
	Vec4                   uvBasis[3];

	IStatObj*              pStatObj;
	phys_geometry*         pPhysGeom;
	uint64                 renderFlags;
};

//! Breakable glass decal shader constants - Sizing, placement and type.
//! Note: Decal structure must be kept in sync with shader (glass.cfx).
struct SBreakableGlassDecalConstant
{
	Vec4 decal; //!< xy = uv-space position, zw = inverse scale
	Vec4 atlas; //!< xy = scaling, zw = position offset
};

//! Runtime control over glass configuration.
struct SBreakableGlassCVars
{
	SBreakableGlassCVars()
		: m_draw(1)
		, m_drawWireframe(0)
		, m_drawDebugData(0)
		, m_drawFragData(0)
		, m_decalAlwaysRebuild(0)
		, m_decalScale(2.5f)
		, m_decalMinRadius(0.25f)
		, m_decalMaxRadius(1.25f)
		, m_decalRandChance(0.67f)
		, m_decalRandScale(1.6f)
		, m_minImpactSpeed(400.0f)
		, m_fragImpulseScale(5.0f)
		, m_fragAngImpulseScale(0.1f)
		, m_fragImpulseDampen(0.3f)
		, m_fragAngImpulseDampen(0.3f)
		, m_fragSpread(1.5f)
		, m_fragMaxSpeed(4.0f)
		, m_impactSplitMinRadius(0.05f)
		, m_impactSplitRandMin(0.5f)
		, m_impactSplitRandMax(1.5f)
		, m_impactEdgeFadeScale(2.0f)
		, m_particleFXEnable(1)
		, m_particleFXUseColours(0)
		, m_particleFXScale(0.25f)
	{
	}

	i32   m_draw;
	i32   m_drawWireframe;
	i32   m_drawDebugData;
	i32   m_drawFragData;

	i32   m_decalAlwaysRebuild;
	float m_decalScale;
	float m_decalMinRadius;
	float m_decalMaxRadius;
	float m_decalRandChance;
	float m_decalRandScale;
	float m_minImpactSpeed;

	float m_fragImpulseScale;
	float m_fragAngImpulseScale;
	float m_fragImpulseDampen;
	float m_fragAngImpulseDampen;
	float m_fragSpread;
	float m_fragMaxSpeed;

	float m_impactSplitMinRadius;
	float m_impactSplitRandMin;
	float m_impactSplitRandMax;
	float m_impactEdgeFadeScale;

	i32   m_particleFXEnable;
	i32   m_particleFXUseColours;
	float m_particleFXScale;
};

//! \endcond