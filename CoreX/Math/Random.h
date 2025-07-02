// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/BaseTypes.h>
#include "LCGRandom.h"
#include "MTPseudoRandom.h"

#include <drx3D/Sys/ISystem.h>

struct SScopedRandomSeedChange
{
	SScopedRandomSeedChange()
	{
		m_prevSeed = gEnv->pSystem->GetRandomGenerator().GetState();
	}

	SScopedRandomSeedChange(u32 seed)
	{
		m_prevSeed = gEnv->pSystem->GetRandomGenerator().GetState();

		gEnv->pSystem->GetRandomGenerator().Seed(seed);
	}

	~SScopedRandomSeedChange()
	{
		gEnv->pSystem->GetRandomGenerator().SetState(m_prevSeed);
	}

	void Seed(u32 seed)
	{
		gEnv->pSystem->GetRandomGenerator().Seed(seed);
	}

	uint64 m_prevSeed;
};

inline CRndGen drx_random_next()
{
	return gEnv->pSystem->GetRandomGenerator().Next();
}

inline u32 drx_random_uint32()
{
	return gEnv->pSystem->GetRandomGenerator().GenerateUint32();
}

//! Scalar ranged random function.
//! Any orderings work correctly: minValue <= maxValue and minValue >= minValue.
//! \return Random value between minValue and maxValue (inclusive).
template<class T>
inline T drx_random(const T minValue, const T maxValue)
{
	return gEnv->pSystem->GetRandomGenerator().GetRandom(minValue, maxValue);
}

//! Vector (Vec2, Vec3, Vec4) ranged random function.
//! All orderings work correctly: minValue.component <= maxValue.component and
//! minValue.component >= maxValue.component.
//! \return A vector, every component of which is between minValue.component and maxValue.component (inclusive).
template<class T>
inline T drx_random_componentwise(const T& minValue, const T& maxValue)
{
	return gEnv->pSystem->GetRandomGenerator().GetRandomComponentwise(minValue, maxValue);
}

//! \return A random unit vector (Vec2, Vec3, Vec4).
template<class T>
inline T drx_random_unit_vector()
{
	return gEnv->pSystem->GetRandomGenerator().GetRandomUnitVector<T>();
}
