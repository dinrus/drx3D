
#ifndef DRX3D_CPU_UTILITY_H
#define DRX3D_CPU_UTILITY_H

#include <drx3D/Maths/Linear/Scalar.h>

#include <string.h>  //memset
#ifdef USE_SIMD
#include <emmintrin.h>
#ifdef DRX3D_ALLOW_SSE4
#include <intrin.h>
#endif  //DRX3D_ALLOW_SSE4
#endif  //USE_SIMD

#if defined DRX3D_USE_NEON
#define ARM_NEON_GCC_COMPATIBILITY 1
#include <arm_neon.h>
#include <sys/types.h>
#include <sys/sysctl.h>  //for sysctlbyname
#endif                   //DRX3D_USE_NEON

///Rudimentary CpuFeatureUtility for CPU features: only report the features that drx3D actually uses (SSE4/FMA3, NEON_HPFP)
///We assume SSE2 in case DRX3D_USE_SSE2 is defined in LinearMathScalar.h
class CpuFeatureUtility
{
public:
	enum CpuFeature
	{
		CPU_FEATURE_FMA3 = 1,
		CPU_FEATURE_SSE4_1 = 2,
		CPU_FEATURE_NEON_HPFP = 4
	};

	static i32 getCpuFeatures()
	{
		static i32 capabilities = 0;
		static bool testedCapabilities = false;
		if (0 != testedCapabilities)
		{
			return capabilities;
		}

#ifdef DRX3D_USE_NEON
		{
			uint32_t hasFeature = 0;
			size_t featureSize = sizeof(hasFeature);
			i32 err = sysctlbyname("hw.optional.neon_hpfp", &hasFeature, &featureSize, NULL, 0);
			if (0 == err && hasFeature)
				capabilities |= CPU_FEATURE_NEON_HPFP;
		}
#endif  //DRX3D_USE_NEON

#ifdef DRX3D_ALLOW_SSE4
		{
			i32 cpuInfo[4];
			memset(cpuInfo, 0, sizeof(cpuInfo));
			zu64 sseExt = 0;
			__cpuid(cpuInfo, 1);

			bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
			bool cpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

			if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
			{
				sseExt = _xgetbv(0);
			}
			i32k OSXSAVEFlag = (1UL << 27);
			i32k AVXFlag = ((1UL << 28) | OSXSAVEFlag);
			i32k FMAFlag = ((1UL << 12) | AVXFlag | OSXSAVEFlag);
			if ((cpuInfo[2] & FMAFlag) == FMAFlag && (sseExt & 6) == 6)
			{
				capabilities |= CpuFeatureUtility::CPU_FEATURE_FMA3;
			}

			i32k SSE41Flag = (1 << 19);
			if (cpuInfo[2] & SSE41Flag)
			{
				capabilities |= CpuFeatureUtility::CPU_FEATURE_SSE4_1;
			}
		}
#endif  //DRX3D_ALLOW_SSE4

		testedCapabilities = true;
		return capabilities;
	}
};

#endif  //DRX3D_CPU_UTILITY_H
