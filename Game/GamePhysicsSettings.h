// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание:	Flags and settings wrappers for physics implementations
							in game. Intention to move all useages of arbitrary physics
							flags in here with game-relevant names.

-------------------------------------------------------------------------
История:
- 04:05:2012: Created by Peter Bottomley

*************************************************************************/

#pragma once

#ifndef __GAMEPHYSICSSETTINGS_H__
#define __GAMEPHYSICSSETTINGS_H__

#include <drx3D/CoreX/Sandbox/IEditorGame.h>

#ifndef _RELEASE
#define GAME_PHYS_DEBUG
#endif //_RELEASE

/*
==========================================================================================
	COLLISION_CLASSES
	Table of Physics Collision Classes - Entity Based Filtering
	Be careful changing these because entities exported in existing levels
	will need to be reexported
==========================================================================================
*/

//========================================================================================
#define COLLISION_CLASSES(f)	     \
	f( collision_class_terrain )     \
	f( collision_class_wheeled )     \
	f( collision_class_living )      \
	f( collision_class_articulated ) \
	f( collision_class_soft )        \
	f( collision_class_particle )    \

//================ ~ COLLISION_CLASSES ===================================================

//========================================================================================
#define GAME_COLLISION_CLASSES(f) \
	f( gcc_player_capsule,     collision_class_game << 0) \
	f( gcc_player_body,        collision_class_game << 1) \
	f( gcc_vehicle,            collision_class_game << 2) \
	f( gcc_large_kickable,     collision_class_game << 3) \
	f( gcc_ragdoll,            collision_class_game << 4) \
	f( gcc_rigid,              collision_class_game << 5) \
	f( gcc_vtol,							 collision_class_game << 6) \
	f( gcc_ai,                 collision_class_game << 7) \


#define GAME_COLLISION_CLASS_COMBOS(f) \
	f( gcc_player_all,								gcc_player_capsule|gcc_player_body) \
	f( gcc_all_engine,								(collision_class_game-1)) \
	f( gcc_all_game,									0xFFFFFFFF&(~gcc_all_engine)) \
	f( gcc_all,												0xFFFFFFFF) \

//================ ~ GAME_COLLISION_CLASSES ==============================================


#define GP_AUTOENUM_PARAM_1_EQUALS_2_COMMA(a,b,...)    a=b,
#define GP_AUTOENUM_BUILD_ENUM_LIST(list) enum { list(GP_AUTOENUM_PARAM_1_EQUALS_2_COMMA) };
GP_AUTOENUM_BUILD_ENUM_LIST(GAME_COLLISION_CLASSES);
GP_AUTOENUM_BUILD_ENUM_LIST(GAME_COLLISION_CLASS_COMBOS);

class CGamePhysicsSettings : public IGamePhysicsSettings
{
public:
	CGamePhysicsSettings() { Init(); }
	~CGamePhysicsSettings(){}

	// IGamePhysicsSettings
	virtual tukk GetCollisionClassName(u32 bitIndex);
	// ~IGamePhysicsSettings

	void Init();
	void ExportToLua();
	i32 GetBit(u32 a);
	void SetIgnoreMap( u32 gcc_classTypes, u32k ignoreClassTypes );
	void AddIgnoreMap( u32 gcc_classTypes, u32k ignoreClassTypesOR, u32k ignoreClassTypesAND = 0xFFFFFFFF );
	void AddCollisionClassFlags( IPhysicalEntity& physEnt, u32 gcc_classTypes, u32k additionalIgnoreClassTypesOR = 0, u32k additionalIgnoreClassTypesAND = 0xFFFFFFFF );
	void SetCollisionClassFlags( IPhysicalEntity& physEnt, u32 gcc_classTypes, u32k additionalIgnoreClassTypesOR = 0, u32k additionalIgnoreClassTypesAND = 0xFFFFFFFF );
	void Debug( const IPhysicalEntity& physEnt, const bool drawAABB ) const;
	i32 ToString( u32 gcc_classTypes, tuk buf, i32k len, const bool trim = true ) const;

private:
	u32 GetIgnoreTypes( u32 gcc_classTypes ) const;

private:
	enum {MAX_COLLISION_CLASSES = 23};   // Lua uses floats, which means any integer using more than 23 bits will be corrupted
	tukk m_names[MAX_COLLISION_CLASSES];
	u32 m_classIgnoreMap[MAX_COLLISION_CLASSES];
};

#endif //__GAMEPHYSICSSETTINGS_H__


