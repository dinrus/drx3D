// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/AI/IAuditionMap.h>

struct IEntityListenerComponent : public IEntityComponent
{
	typedef std::function<bool(const Perception::SSoundStimulusParams&)> UserConditionCallback;
	typedef std::function<void(const Perception::SSoundStimulusParams&)> SoundHeardCallback;
	
	virtual void SetUserConditionCallback(const UserConditionCallback& callback) = 0;
	virtual void SetSoundHeardCallback(const SoundHeardCallback& callback) = 0;

	static void ReflectType(sxema::CTypeDesc<IEntityListenerComponent>& desc)
	{
		desc.SetGUID("E7168B1B-E08F-43B0-BD10-0C48DE89FFB8"_drx_guid);
	}
};