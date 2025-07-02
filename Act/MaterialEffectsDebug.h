// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// ----------------------------------------------------------------------------------------
//  Имя файла:   MaterialEffectsDebug.h
//  Описание: MaterialEffects debug utility class
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MATERIAL_EFFECTS_DEBUG_H_
#define _MATERIAL_EFFECTS_DEBUG_H_

#pragma once

#include <drx3D/Act/IMaterialEffects.h>

class CMaterialEffects;

namespace MaterialEffectsUtils
{

#ifdef MATERIAL_EFFECTS_DEBUG

class CVisualDebug
{
private:

	struct SDebugVisualEntry
	{
		SDebugVisualEntry()
			: lifeTime(0.0f)
			, fxId(InvalidEffectId)
			, materialName1("")
			, materialName2("")
		{

		}

		DrxFixedStringT<32> materialName1;
		DrxFixedStringT<32> materialName2;

		Vec3                fxPosition;
		Vec3                fxDirection;

		float               lifeTime;

		TMFXEffectId        fxId;
	};

	struct SLastSearchHint
	{
		SLastSearchHint()
		{
			Reset();
		}

		void Reset()
		{
			materialName1 = "";
			materialName2 = "";
			fxId = InvalidEffectId;
		}

		DrxFixedStringT<32> materialName1;
		DrxFixedStringT<32> materialName2;

		TMFXEffectId        fxId;
	};

public:

	CVisualDebug()
		: m_nextHit(0)
	{

	}

	void AddLastSearchHint(const TMFXEffectId effectId, i32k surfaceIndex1, i32k surfaceIndex2);
	void AddLastSearchHint(const TMFXEffectId effectId, tukk customName, i32k surfaceIndex2);
	void AddLastSearchHint(const TMFXEffectId effectId, const IEntityClass* pEntityClass, i32k surfaceIndex2);

	void AddEffectDebugVisual(const TMFXEffectId effectId, const SMFXRunTimeEffectParams& runtimeParams);
	void Update(const CMaterialEffects& materialEffects, const float frameTime);

private:

	const static u32 kMaxDebugVisualMfxEntries = 48;

	SLastSearchHint     m_lastSearchHint;

	SDebugVisualEntry   m_effectList[kMaxDebugVisualMfxEntries];
	u32              m_nextHit;
};

#endif //MATERIAL_EFFECTS_DEBUG

}

#endif // _MATERIAL_EFFECTS_DEBUG_H_
