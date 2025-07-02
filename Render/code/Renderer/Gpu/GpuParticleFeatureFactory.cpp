// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/GpuParticleFeatureFactory.h>

#include <drx3D/Render/GpuParticleFeatureColor.h>
#include <drx3D/Render/GpuParticleFeatureCollision.h>
#include <drx3D/Render/GpuParticleFeatureField.h>
#include <drx3D/Render/GpuParticleFeatureMotion.h>
#include <drx3D/Render/GpuParticleFeatureFluidDynamics.h>

namespace gpu_pfx2
{

// this dummy is used when a feature is supported on GPU but does not
// need any special treatment (e.g. all the appearance features
struct CFeatureDummy : public CFeature
{
	static const EGpuFeatureType type = eGpuFeatureType_Dummy;
};

CGpuInterfaceFactory::CGpuInterfaceFactory()
{
	memset(m_functions, 0, sizeof(m_functions));
#define X(name) RegisterClass<CFeature ## name>();
	LIST_OF_FEATURE_TYPES
#undef X
}

}
