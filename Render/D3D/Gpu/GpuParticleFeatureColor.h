// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     02/10/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Render/GpuParticleFeatureBase.h>
#include <drx3D/Render/GpuComputeBackend.h>

namespace gpu_pfx2
{

struct CFeatureColor : public CFeature
{
	static const EGpuFeatureType type = eGpuFeatureType_Color;

	CFeatureColor() : m_colorTable(kNumModifierSamples) {}
	virtual void Initialize() override;
	virtual void InitParticles(const SUpdateContext& context) override;
	virtual void Update(const gpu_pfx2::SUpdateContext& context, CDeviceCommandListRef RESTRICT_REFERENCE commandList) override;
	virtual void InternalSetParameters(const EParameterType type, const SFeatureParametersBase& p) override;
private:
	gpu::CStructuredResource<Vec3, gpu::BufferFlagsDynamic> m_colorTable;
};

}
