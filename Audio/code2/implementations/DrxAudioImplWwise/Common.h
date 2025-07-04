// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/AK/AkWwiseSDKVersion.h>

#if AK_WWISESDK_VERSION_MAJOR <= 2017 && AK_WWISESDK_VERSION_MINOR < 2
	#error This version of Wwise is not supported, the minimum supported version is 2017.2.0
#endif

#include <drx3D/Audio/AK/SoundEngine/Common/AkTypes.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>

#define WWISE_IMPL_INFO_STRING "Wwise " AK_WWISESDK_VERSIONNAME

#define ASSERT_WWISE_OK(x) (DRX_ASSERT(x == AK_Success))
#define IS_WWISE_OK(x)     (x == AK_Success)

namespace DrxAudio
{
namespace Impl
{
namespace Wwise
{
//////////////////////////////////////////////////////////////////////////
inline void FillAKVector(Vec3 const& vDrxVector, AkVector& vAKVector)
{
	vAKVector.X = vDrxVector.x;
	vAKVector.Y = vDrxVector.z;
	vAKVector.Z = vDrxVector.y;
}

///////////////////////////////////////////////////////////////////////////
inline void FillAKObjectPosition(CObjectTransformation const& transformation, AkSoundPosition& outTransformation)
{
	AkVector vec1, vec2;
	FillAKVector(transformation.GetPosition(), vec1);
	outTransformation.SetPosition(vec1);
	FillAKVector(transformation.GetForward(), vec1);
	FillAKVector(transformation.GetUp(), vec2);
	outTransformation.SetOrientation(vec1, vec2);
}

///////////////////////////////////////////////////////////////////////////
inline void FillAKListenerPosition(CObjectTransformation const& transformation, AkListenerPosition& outTransformation)
{
	AkVector vec1, vec2;
	FillAKVector(transformation.GetPosition(), vec1);
	outTransformation.SetPosition(vec1);
	FillAKVector(transformation.GetForward(), vec1);
	FillAKVector(transformation.GetUp(), vec2);
	outTransformation.SetOrientation(vec1, vec2);
}

extern AkGameObjectID g_listenerId; // To be removed once multi-listener support is implemented.
extern AkGameObjectID g_globalObjectId;
} //endns Wwise
} //endns Impl
} //endns DrxAudio
