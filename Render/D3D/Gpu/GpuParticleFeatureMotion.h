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

namespace gpu_pfx2
{

struct CLocalEffector
{
	virtual ~CLocalEffector() {};
	virtual void Update(const gpu_pfx2::SUpdateContext& context) = 0;
	virtual void Initialize() = 0;
};

template<class ConstantBufferStruct, const EConstantBufferSlot constantBufferSlot, u32k updateFlag, const EParameterType type>
struct CEffectorBase : public CLocalEffector
{
	typedef ConstantBufferStruct TConstantBufferStruct;
	static const EParameterType parameterType = type;

	CEffectorBase(ConstantBufferStruct params)
	{
		m_parameters = params;
	}
	virtual void Update(const gpu_pfx2::SUpdateContext& context)
	{
		CParticleComponentRuntime* pRuntime = static_cast<CParticleComponentRuntime*>(context.pRuntime);
		pRuntime->SetUpdateFlags(updateFlag);
		m_parameters.CopyToDevice();
		pRuntime->SetUpdateConstantBuffer(constantBufferSlot, m_parameters.GetDeviceConstantBuffer());
	}
	virtual void Initialize()
	{
		if (!m_parameters.IsDeviceBufferAllocated())
		{
			m_parameters.CreateDeviceBuffer();
		}
	}
private:
	gpu::CTypedConstantBuffer<ConstantBufferStruct> m_parameters;
};

typedef CEffectorBase<SFeatureParametersMotionPhysicsBrownian, eConstantBufferSlot_Brownian, eFeatureUpdateFlags_Motion_Brownian, EParameterType::MotionPhysicsBrownian> TBrownianEffector;
typedef CEffectorBase<SFeatureParametersMotionPhysicsSimplex, eConstantBufferSlot_Simplex, eFeatureUpdateFlags_Motion_Simplex, EParameterType::MotionPhysicsSimplex>     TSimplexEffector;
typedef CEffectorBase<SFeatureParametersMotionPhysicsCurl, eConstantBufferSlot_Curl, eFeatureUpdateFlags_Motion_Curl, EParameterType::MotionPhysicsCurl>                 TCurlEffector;
typedef CEffectorBase<SFeatureParametersMotionPhysicsGravity, eConstantBufferSlot_Gravity, eFeatureUpdateFlags_Motion_Gravity, EParameterType::MotionPhysicsGravity>     TGravityEffector;
typedef CEffectorBase<SFeatureParametersMotionPhysicsVortex, eConstantBufferSlot_Vortex, eFeatureUpdateFlags_Motion_Vortex, EParameterType::MotionPhysicsVortex>         TVortexEffector;

struct CFeatureMotion : public CFeature
{
	static const EGpuFeatureType type = eGpuFeatureType_Motion;
	static i32k             kNumberOfEffectors = 5;

	enum EIntegrator
	{
		EI_Linear,
		EI_DragFast,
	};

	virtual void Update(const gpu_pfx2::SUpdateContext& context, CDeviceCommandListRef RESTRICT_REFERENCE commandList) override;
	virtual void Initialize() override;
	virtual void InternalSetParameters(const EParameterType type, const SFeatureParametersBase& p) override;
private:
	gpu::CTypedConstantBuffer<SFeatureParametersMotionPhysics> m_parameters;
	std::unique_ptr<CLocalEffector>                            m_pEffectors[kNumberOfEffectors];
	EIntegrator m_integrator;
};

}
