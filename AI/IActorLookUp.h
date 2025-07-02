// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IAIObject.h>
#include <drx3D/AI/IAIActor.h>

template<typename Ty> struct ActorLookUpCastHelper
{
	static Ty* Cast(IAIActor* ptr) { assert(!"dangerous cast!"); return static_cast<Ty*>(ptr); }
};

template<> struct ActorLookUpCastHelper<IAIObject>
{
	static IAIObject* Cast(IAIActor* ptr) { return ptr->CastToIAIObject(); }
};

template<> struct ActorLookUpCastHelper<IAIActor>
{
	static IAIActor* Cast(IAIActor* ptr) { return ptr; }
};

struct IAIActorProxy;

struct IActorLookUp
{
public:
	enum
	{
		Proxy = BIT(0),
		Position = BIT(1),
		EntityID = BIT(2),
	};

	virtual ~IActorLookUp() {}
	virtual size_t GetActiveCount() const = 0;

	template<typename Ty>
	Ty* GetActor(u32 index) const
	{
		IAIActor* actor = GetActorInternal(index);
		if (actor)
		{
			return ActorLookUpCastHelper<Ty>::Cast(actor);
		}
		return nullptr;
	}

	virtual IAIActorProxy* GetProxy(u32 index) const = 0;
	virtual const Vec3& GetPosition(u32 index) const = 0;
	virtual EntityId GetEntityID(u32 index) const = 0;
	virtual void UpdatePosition(CAIActor* actor, const Vec3& position) = 0;
	virtual void UpdateProxy(CAIActor* actor) = 0;
	virtual void Prepare(u32 lookUpFields) = 0;

	virtual void AddActor(CAIActor* actor) = 0;
	virtual void RemoveActor(CAIActor* actor) = 0;

private:
	virtual IAIActor* GetActorInternal(u32 index) const = 0;
};
