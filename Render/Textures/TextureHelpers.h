// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   TextureHelpers.h
//  Version:     v1.00
//  Created:     12/6/2014 by NielsF.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2012
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TextureHelpers_h__
#define __TextureHelpers_h__
#pragma once

#include <drx3D/Render/Texture.h>

namespace TextureHelpers
{
//////////////////////////////////////////////////////////////////////////
EEfResTextures FindTexSlot(tukk texSemantic);
bool           VerifyTexSuffix(EEfResTextures texSlot, tukk texPath);
bool           VerifyTexSuffix(EEfResTextures texSlot, const string& texPath);
tukk    LookupTexSuffix(EEfResTextures texSlot);
int8           LookupTexPriority(EEfResTextures texSlot);
CTexture*      LookupTexDefault(EEfResTextures texSlot);
CTexture*      LookupTexNeutral(EEfResTextures texSlot);
}

#endif
