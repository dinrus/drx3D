// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Created:     02/10/2015 by Benjamin Block
//  Описание:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <drx3D/Render/GpuParticleFeatureBase.h>
#include <drx3D/Render/GpuParticleComponentRuntime.h>
#include <drx3D/Render/GpuComputeBackend.h>
#include <drx3D/Render/GpuParticleComponentRuntime.h>

namespace gpu_pfx2
{

struct CFeatureFieldOpacity : public CFeature
{
	static const EGpuFeatureType type = eGpuFeatureType_FieldOpacity;

	CFeatureFieldOpacity() : m_opacityTable(kNumModifierSamples) {}
	virtual void Initialize() override;
	virtual void InitParticles(const SUpdateContext& context) override;
	virtual void Update(const gpu_pfx2::SUpdateContext& context, CDeviceCommandListRef RESTRICT_REFERENCE commandList) override;
	virtual void InternalSetParameters(const EParameterType type, const SFeatureParametersBase& p) override;

private:
	gpu::CStructuredResource<float, gpu::BufferFlagsDynamic> m_opacityTable;
};

struct CFeatureFieldSize : public CFeature
{
	static const EGpuFeatureType type = eGpuFeatureType_FieldSize;

	CFeatureFieldSize() : m_sizeTable(kNumModifierSamples) {}
	virtual void Initialize() override;
	virtual void InitParticles(const SUpdateContext& context) override;
	virtual void Update(const gpu_pfx2::SUpdateContext& context, CDeviceCommandListRef RESTRICT_REFERENCE commandList) override;
	virtual void InternalSetParameters(const EParameterType type, const SFeatureParametersBase& p) override;

private:
	gpu::CStructuredResource<float, gpu::BufferFlagsDynamic> m_sizeTable;
};

struct SFeatureInternalParametersPixelSize
{
	Vec4  projectionPlane;
	float minPixelSize;
	float invMinPixelSize;
	float maxPixelSize;
	float minDrawPixels;
	i32 affectOpacity;
};

struct CFeatureFieldPixelSize : public CFeatureWithParameterStruct<SFeatureParametersPixelSize>
{
	static const EGpuFeatureType type = eGpuFeatureType_FieldPixelSize;

	CFeatureFieldPixelSize()  {}
	virtual void Initialize() override;
	virtual void InitParticles(const SUpdateContext& context) override;
	virtual void Update(const gpu_pfx2::SUpdateContext& context, CDeviceCommandListRef RESTRICT_REFERENCE commandList) override;
private:
	gpu::CTypedConstantBuffer<SFeatureInternalParametersPixelSize> m_internalParameters;
};
}
