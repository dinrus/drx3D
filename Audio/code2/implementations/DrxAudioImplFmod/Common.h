// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/SharedAudioData.h>
#include <fmod_common.h>

#define FMOD_IMPL_INFO_STRING             "Fmod Studio - "

#define ASSERT_FMOD_OK                    DRX_ASSERT(fmodResult == FMOD_OK)
#define ASSERT_FMOD_OK_OR_INVALID_HANDLE  DRX_ASSERT(fmodResult == FMOD_OK || fmodResult == FMOD_ERR_INVALID_HANDLE)
#define ASSERT_FMOD_OK_OR_NOT_LOADED      DRX_ASSERT(fmodResult == FMOD_OK || fmodResult == FMOD_ERR_STUDIO_NOT_LOADED)
#define ASSERT_FMOD_OK_OR_EVENT_NOT_FOUND DRX_ASSERT(fmodResult == FMOD_OK || fmodResult == FMOD_ERR_EVENT_NOTFOUND)
#define FMOD_IMPL_INVALID_INDEX           (-1)

namespace DrxAudio
{
namespace Impl
{
namespace Fmod
{
///////////////////////////////////////////////////////////////////////////
inline void FillFmodObjectPosition(SObject3DAttributes const& inAttributes, FMOD_3D_ATTRIBUTES& outAttributes)
{
	outAttributes.forward.x = inAttributes.transformation.GetForward().x;
	outAttributes.forward.z = inAttributes.transformation.GetForward().y;
	outAttributes.forward.y = inAttributes.transformation.GetForward().z;

	outAttributes.position.x = inAttributes.transformation.GetPosition().x;
	outAttributes.position.z = inAttributes.transformation.GetPosition().y;
	outAttributes.position.y = inAttributes.transformation.GetPosition().z;

	outAttributes.up.x = inAttributes.transformation.GetUp().x;
	outAttributes.up.z = inAttributes.transformation.GetUp().y;
	outAttributes.up.y = inAttributes.transformation.GetUp().z;

	outAttributes.velocity.x = inAttributes.velocity.x;
	outAttributes.velocity.z = inAttributes.velocity.y;
	outAttributes.velocity.y = inAttributes.velocity.z;
}
}
}
}
