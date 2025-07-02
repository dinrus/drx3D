// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Sys/IEngineModule.h>

#if defined (_LIB)
#define DRXPHYSICS_API
#elif defined (PHYSICS_EXPORTS)
#define DRXPHYSICS_API DLL_EXPORT
#else
#define DRXPHYSICS_API DLL_IMPORT
#endif

struct IPhysicalWorld;

#if !defined (_LIB)
extern "C"
#endif
DRXPHYSICS_API IPhysicalWorld * CreatePhysicalWorld(struct ISystem* pLog);

//! IDs that can be used for foreign id.
enum EPhysicsForeignIds
{
	PHYS_FOREIGN_ID_TERRAIN                  = 0,
	PHYS_FOREIGN_ID_STATIC                   = 1,
	PHYS_FOREIGN_ID_ENTITY                   = 2,
	PHYS_FOREIGN_ID_FOLIAGE                  = 3,
	PHYS_FOREIGN_ID_ROPE                     = 4,
	PHYS_FOREIGN_ID_SOUND_OBSTRUCTION        = 5,
	PHYS_FOREIGN_ID_SOUND_PROXY_OBSTRUCTION  = 6,
	PHYS_FOREIGN_ID_SOUND_REVERB_OBSTRUCTION = 7,
	PHYS_FOREIGN_ID_WATERVOLUME              = 8,
	PHYS_FOREIGN_ID_BREAKABLE_GLASS          = 9,
	PHYS_FOREIGN_ID_BREAKABLE_GLASS_FRAGMENT = 10,
	PHYS_FOREIGN_ID_RIGID_PARTICLE           = 11,
	PHYS_FOREIGN_ID_RESERVED1                = 12,
	PHYS_FOREIGN_ID_RAGDOLL                  = 13,

	PHYS_FOREIGN_ID_USER                     = 100, //!< All user defined foreign ids should start from this enum.
};

struct IPhysicsEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IPhysicsEngineModule, "00000b61-97f3-3c68-a134-08bb46801741"_drx_guid);
};

#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <drx3D/Phys/physinterface.h>
