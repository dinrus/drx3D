// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Eng3D/ParticleCommon.h>
#include <drx3D/Eng3D/ParticleAttributes.h>
#include <drx3D/Eng3D/ParticleEmitter.h>
#include <drx3D/Eng3D/ParticleComponentRuntime.h>
#include <drx3D/Eng3D/ParamMod.h>

namespace pfx2
{

SERIALIZATION_ENUM_DECLARE(ETargetSource, ,
                           Self,
                           Parent,
                           Target
                           )

// PFX2_TODO : optimize : GetTarget is very inefficient. Make static dispatch and then vectorize it.

class CTargetSource
{
public:
	CTargetSource(ETargetSource source = ETargetSource::Target);

	void Serialize(Serialization::IArchive& ar);
	Vec3 GetTarget(const CParticleComponentRuntime& runtime, TParticleId particleId, bool isParentId = false);

private:
	Vec3          m_offset;
	ETargetSource m_source;
};

}

#include "TargetImpl.h"
