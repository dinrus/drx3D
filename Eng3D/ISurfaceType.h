// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ISurfaceType.h
//  Version:     v1.00
//  Created:     30/9/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:   Defines interfaces to access Surface Types.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

//! Flags that ISurfaceType::GetFlags() can return.
enum ESurfaceTypeFlags
{
	SURFACE_TYPE_NO_PHYSICALIZE         = BIT(1), //!< This surface should not be physicalized.
	SURFACE_TYPE_NO_COLLIDE             = BIT(2), //!< Should only be set for vegetation canopy mats.
	SURFACE_TYPE_VEHICLE_ONLY_COLLISION = BIT(3),
	SURFACE_TYPE_CAN_SHATTER            = BIT(4), //!< This surface type can shatter.
	SURFACE_TYPE_DRX3D_PIERCEABLE      = BIT(5), //!< This surface is pierceable by bullets (used by MFX system to spawn front/back FX).
};

//! \cond INTERNAL
//! Parameter structure passed to ISurfaceType::Execute.
struct SSurfaceTypeExecuteParams
{
	Vec3 hitPoint;
	Vec3 hitNormal;
	i32  hitType;
};
//! \endcond

#define SURFACE_BREAKAGE_TYPE(x) x

//! Surface definition.
struct ISurfaceType
{
	//////////////////////////////////////////////////////////////////////////
	struct SSurfaceTypeAIParams
	{
		float fImpactRadius;
		float fImpactSoundRadius;
		float fFootStepRadius;
		float proneMult;
		float crouchMult;
		float movingMult;

		SSurfaceTypeAIParams()
		{
			fImpactRadius = 2.5f;
			fImpactSoundRadius = 20.0f;
			fFootStepRadius = 20.0f;
			proneMult = 0.2f;
			crouchMult = 0.5f;
			movingMult = 2.5f;
		}
	};
	struct SPhysicalParams
	{
		i32   breakable_id;
		i32   break_energy;
		float hole_size;
		float hole_size_explosion;
		float hit_radius;
		float hit_points;
		float hit_points_secondary;
		float hit_maxdmg;
		float hit_lifetime;
		i32   pierceability;
		float damage_reduction;
		float ric_angle;
		float ric_dam_reduction;
		float ric_vel_reduction;
		float friction;
		float bouncyness;
		i32   iBreakability;
		i32   collType;
		float sound_obstruction;
	};
	struct SBreakable2DParams
	{
		string particle_effect;
		float  blast_radius;
		float  blast_radius_first;
		float  vert_size_spread;
		i32    rigid_body;
		float  life_time;
		float  cell_size;
		i32    max_patch_tris;
		float  filter_angle;
		float  shard_density;
		i32    use_edge_alpha;
		float  crack_decal_scale;
		string crack_decal_mtl;
		float  max_fracture;
		string full_fracture_fx;
		string fracture_fx;
		i32    no_procedural_full_fracture;
		string broken_mtl;
		float  destroy_timeout;
		float  destroy_timeout_spread;

		SBreakable2DParams() : blast_radius(0), rigid_body(0), life_time(0), cell_size(0), max_patch_tris(0), shard_density(0), crack_decal_scale(0),
			max_fracture(1.0f), vert_size_spread(0), filter_angle(0), use_edge_alpha(0), blast_radius_first(0), no_procedural_full_fracture(0),
			destroy_timeout(0), destroy_timeout_spread(0) {}
	};
	struct SBreakageParticles
	{
		string type;
		string particle_effect;
		i32    count_per_unit;
		float  count_scale;
		float  scale;
		SBreakageParticles() : count_per_unit(1), count_scale(1), scale(1) {}
	};

	// <interfuscator:shuffle>
	virtual ~ISurfaceType(){}

	//! Releases surface type.
	virtual void Release() = 0;

	//! Return unique Id of this surface type.
	//! Maximum of 65535 simultanious surface types can exist.
	virtual u16 GetId() const = 0;

	//! Unique name of the surface type.
	virtual tukk GetName() const = 0;

	//! Typename of this surface type.
	virtual tukk GetType() const = 0;

	//! Flags of the surface type.
	//! \return Combination of ESurfaceTypeFlags flags.
	virtual i32 GetFlags() const = 0;

	//! Execute material.
	virtual void Execute(SSurfaceTypeExecuteParams& params) = 0;

	//! Returns a some cached properties for faster access.
	virtual i32   GetBreakability() const = 0;
	virtual float GetBreakEnergy() const = 0;
	virtual i32   GetHitpoints() const = 0;

	//////////////////////////////////////////////////////////////////////////
	virtual const SPhysicalParams& GetPhyscalParams() = 0;

	//! Optional AI Params.
	virtual const SSurfaceTypeAIParams* GetAIParams() = 0;

	//! Optional params for 2D breakable plane.
	virtual SBreakable2DParams* GetBreakable2DParams() = 0;
	virtual SBreakageParticles* GetBreakageParticles(tukk sType, bool bLookInDefault = true) = 0;

	// Called by Surface manager.

	//! Loads surface, (do not use directly).
	virtual bool Load(i32 nId) = 0;
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
// Описание:
//    This interface is used to enumerate all items registered to the surface type manager.
//////////////////////////////////////////////////////////////////////////
struct ISurfaceTypeEnumerator
{
	// <interfuscator:shuffle>
	virtual ~ISurfaceTypeEnumerator(){}
	virtual void          Release() = 0;
	virtual ISurfaceType* GetFirst() = 0;
	virtual ISurfaceType* GetNext() = 0;
	// </interfuscator:shuffle>
};

// Описание:
//    Manages loading and mapping of physical surface materials to Ids and materials scripts.
// Behaviour:
//    At start will enumerate all material names.
//    When the surface is first time requested by name it will be loaded and cached
//    and new unique id will be generated for it.
struct ISurfaceTypeUpr
{
	// <interfuscator:shuffle>
	virtual ~ISurfaceTypeUpr(){}

	//! Load Surface types
	virtual void LoadSurfaceTypes() = 0;

	//! Return surface type by name.
	//! If surface is not yet loaded it will be loaded and and cached.
	//! \param sName Name of the surface type ("mat_metal","mat_wood", etc..).
	//! \param warn Print warning message if surface not found.
	virtual ISurfaceType* GetSurfaceTypeByName(tukk sName, tukk sWhy = NULL, bool warn = true) = 0;

	//! Return surface type by id.
	//! If surface is not yet loaded it will be loaded and and cached.
	//! \param sName Name of the surface type ("mat_metal","mat_wood", etc..).
	virtual ISurfaceType* GetSurfaceType(i32 nSurfaceId, tukk sWhy = NULL) = 0;

	//! Retrieve an interface to the enumerator class that allow to iterate over all surface types.
	virtual ISurfaceTypeEnumerator* GetEnumerator() = 0;

	//! Register a new surface type.
	virtual bool RegisterSurfaceType(ISurfaceType* pSurfaceType, bool bDefault = false) = 0;
	virtual void UnregisterSurfaceType(ISurfaceType* pSurfaceType) = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	// </interfuscator:shuffle>
};
