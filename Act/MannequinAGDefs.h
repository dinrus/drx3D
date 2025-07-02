// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//-------------------------------------------------------------------------
//
// Описание:
//  Basic type definitions used by the DrxMannequin AnimationGraph adapter
//  classes
//
////////////////////////////////////////////////////////////////////////////
#ifndef __MANNEQUINAGDEFS_H__
#define __MANNEQUINAGDEFS_H__
#pragma once

namespace MannequinAG
{

// ============================================================================
// ============================================================================

typedef DrxFixedStringT<64> TKeyValue;

enum ESupportedInputID
{
	eSIID_Action = 0, // has to start with zero (is an assumption all over the place).,
	eSIID_Signal,

	eSIID_COUNT,
};

enum EAIActionType
{
	EAT_Looping,
	EAT_OneShot,
};
// ============================================================================
// ============================================================================

} //endns MannequinAG

#endif // __MANNEQUINAGDEFS_H__
