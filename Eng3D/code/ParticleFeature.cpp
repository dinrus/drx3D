// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     01/10/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ParticleFeature.h>
#include <drx3D/Eng3D/ParticleSystem.h>

namespace pfx2
{

bool CParticleFeature::RegisterFeature(const SParticleFeatureParams& params)
{
	return CParticleSystem::RegisterFeature(params);
}

void CParticleFeature::Serialize(Serialization::IArchive& ar)
{
	ar(m_enabled);
}

gpu_pfx2::IParticleFeature* CParticleFeature::MakeGpuInterface(CParticleComponent* pComponent, gpu_pfx2::EGpuFeatureType feature)
{
	if (!feature || !pComponent->UsesGPU() || !gEnv->pRenderer)
		m_gpuInterface.reset();
	else if (!m_gpuInterface)
		m_gpuInterface = gEnv->pRenderer->GetGpuParticleUpr()->CreateParticleFeature(feature);
	pComponent->AddGPUFeature(m_gpuInterface);
	return m_gpuInterface;
}

}
