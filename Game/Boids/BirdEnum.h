// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   birdEnum.h
//  Version:     v1.00
//  Created:     7/2010 by Luciano Morpurgo
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 7.0
//  Описание: Bird Namespace with enums and others
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __birdenum_h__
#define __birdenum_h__

#if _MSC_VER > 1000
#pragma once
#endif


namespace Bird
{
	typedef enum
	{
		FLYING,
		TAKEOFF,
		LANDING,
		ON_GROUND,
		DEAD
	} EStatus;

	// sub-stati of EStatus::ON_GROUND
	typedef enum
	{
		OGS_WALKING,
		OGS_SLOWINGDOWN,	// transitioning from walking to idle
		OGS_IDLE
	} EOnGroundStatus;

	enum
	{
		ANIM_FLY,
		ANIM_TAKEOFF,
		ANIM_WALK,
		ANIM_IDLE,
		ANIM_LANDING_DECELERATING,	// we've been in the landing state for a while and are now decelerating to land softly on the ground
	};

} //endns


#endif // __birdenum_h__
