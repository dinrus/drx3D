// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "JoystickUtils.h"
#include <DrxInput/IJoystick.h>
#include <drx3D/CoreX/Math/ISplines.h>
#include <DrxAnimation/IFacialAnimation.h>

float JoystickUtils::Evaluate(IJoystickChannel* pChannel, float time)
{
	float total = 0.0f;
	for (i32 splineIndex = 0, splineCount = (pChannel ? i32(pChannel->GetSplineCount()) : 0); splineIndex < splineCount; ++splineIndex)
	{
		ISplineInterpolator* pSpline = (pChannel ? pChannel->GetSpline(splineIndex) : 0);
		float v = 0.0f;
		if (pSpline)
			pSpline->InterpolateFloat(time, v);
		total += v;
	}
	return total;
}

void JoystickUtils::SetKey(IJoystickChannel* pChannel, float time, float value, bool createIfMissing)
{
	// Add up all the splines except the last one.
	float total = 0.0f;
	for (i32 splineIndex = 0, splineCount = (pChannel ? i32(pChannel->GetSplineCount()) : 0); splineIndex < splineCount - 1; ++splineIndex)
	{
		ISplineInterpolator* pSpline = (pChannel ? pChannel->GetSpline(splineIndex) : 0);
		float v = 0.0f;
		if (pSpline)
			pSpline->InterpolateFloat(time, v);
		total += v;
	}

	// Set the key on the last spline so that the total value equals the required value.
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	i32 keyIndex = (interpolator ? interpolator->FindKey(FacialEditorSnapTimeToFrame(time), 1.0e-5f) : -1);
	if (interpolator && keyIndex < 0)
	{
		if (createIfMissing)
			interpolator->InsertKeyFloat(FacialEditorSnapTimeToFrame(time), value - total);
	}
	else if (interpolator)
	{
		interpolator->SetKeyValueFloat(keyIndex, value - total);
	}
}

void JoystickUtils::Serialize(IJoystickChannel* pChannel, XmlNodeRef node, bool bLoading)
{
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	if (interpolator)
		interpolator->SerializeSpline(node, bLoading);
}

bool JoystickUtils::HasKey(IJoystickChannel* pChannel, float time)
{
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	i32 keyIndex = (interpolator ? interpolator->FindKey(FacialEditorSnapTimeToFrame(time), 1.0e-5f) : -1);
	return (keyIndex >= 0);
}

void JoystickUtils::RemoveKey(IJoystickChannel* pChannel, float time)
{
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	i32 keyIndex = (interpolator ? interpolator->FindKey(FacialEditorSnapTimeToFrame(time), 1.0e-5f) : -1);
	if (interpolator)
		interpolator->RemoveKey(keyIndex);
}

void JoystickUtils::RemoveKeysInRange(IJoystickChannel* pChannel, float startTime, float endTime)
{
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	if (interpolator)
		interpolator->RemoveKeysInRange(startTime, endTime);
}

void JoystickUtils::PlaceKey(IJoystickChannel* pChannel, float time)
{
	float frameTime = FacialEditorSnapTimeToFrame(time);
	i32 splineCount = (pChannel ? pChannel->GetSplineCount() : 0);
	ISplineInterpolator* interpolator = (pChannel && splineCount >= 0 ? pChannel->GetSpline(splineCount - 1) : 0);
	if (pChannel && interpolator && !HasKey(pChannel, frameTime))
	{
		float value;
		interpolator->InterpolateFloat(frameTime, value);
		interpolator->InsertKeyFloat(frameTime, value);
	}
}

