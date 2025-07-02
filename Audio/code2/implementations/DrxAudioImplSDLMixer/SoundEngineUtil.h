// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SoundEngine.h"
#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/Sys/DrxFile.h>
#include <drx3D/CoreX/String/Path.h>

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
inline const SampleId GetIDFromString(const string& name)
{
	return CCrc32::ComputeLowercase(name.c_str());
}

inline const SampleId GetIDFromString(char const* const szName)
{
	return CCrc32::ComputeLowercase(szName);
}

inline void GetDistanceAngleToObject(const CObjectTransformation& listener, const CObjectTransformation& object, float& out_distance, float& out_angle)
{
	const Vec3 listenerToObject = object.GetPosition() - listener.GetPosition();

	// Distance
	out_distance = listenerToObject.len();

	// Angle
	// Project point to plane formed by the listeners position/direction
	Vec3 n = listener.GetUp().GetNormalized();
	Vec3 objectDir = Vec3::CreateProjection(listenerToObject, n).normalized();

	// Get angle between listener position and projected point
	const Vec3 listenerDir = listener.GetForward().GetNormalizedFast();
	out_angle = RAD2DEG(asin_tpl(objectDir.Cross(listenerDir).Dot(n)));
}
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
