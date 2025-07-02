// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001-2012
// -------------------------------------------------------------------------
//  File name:   AnimUtils.h
//  Created:     9/11/2006 by Michael S.
//  Description: Animation utilities
//
////////////////////////////////////////////////////////////////////////////
struct ICharacterInstance;

namespace AnimUtils
{
void StartAnimation(ICharacterInstance* pCharacter, tukk pAnimName);
void SetAnimationTime(ICharacterInstance* pCharacter, float fNormalizedTime);
void StopAnimations(ICharacterInstance* pCharacter);
}

