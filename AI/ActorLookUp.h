// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ActorLookUp_h__
#define __ActorLookUp_h__

#pragma once

#include <drx3D/AI/IActorLookUp.h>
#include <drx3D/AI/AIActor.h>

template<> struct ActorLookUpCastHelper<CAIObject>
{
	static CAIObject* Cast(IAIActor* ptr) { return static_cast<CAIActor*>(ptr); }
};

template<> struct ActorLookUpCastHelper<CAIActor>
{
	static CAIActor* Cast(IAIActor* ptr) { return  static_cast<CAIActor*>(ptr); }
};

template<> struct ActorLookUpCastHelper<CPipeUser>
{
	static CPipeUser* Cast(IAIActor* ptr) { return static_cast<CAIActor*>(ptr)->CastToCPipeUser(); }
};

template<> struct ActorLookUpCastHelper<CPuppet>
{
	static CPuppet* Cast(IAIActor* ptr) { return static_cast<CAIActor*>(ptr)->CastToCPuppet(); }
};

class ActorLookUp : public IActorLookUp
//Класс ПоискАктора, наследует от интерфейса ИПоискАктора
{
public:
	virtual size_t GetActiveCount() const override
	//Виртуальная функция дайАктивныйСчёт, возврзнач типа т_мера
	{
		return m_actors.size();
	}

	virtual IAIActorProxy* GetProxy(u32 index) const override
	//Виртуальная функция дайПрокси, возврзнач типа ИПроксиАктораИИ, принимает бцел32 "индекс"
	{
		return m_proxies[index];
	}

	virtual const Vec3& GetPosition(u32 index) const override
	//Виртуальная функция дайПозицию, возврзнач типа ссылка на конст Век3, принимает бцел32
	//"индекс"
	{
		return m_positions[index];
	}

	virtual EntityId GetEntityID(u32 index) const override
	//Виртуальная функция дайИДСущности, возврзнач ИдСущности, принимает бцел32 "индекс"
	{
		return m_entityIDs[index];
	}

	virtual void UpdatePosition(CAIActor* actor, const Vec3& position) override
	//Виртуальный метод обновиПозицию, принимает ук на АкторИИ, и ссылку на конст Век3 "позиция"
	{
		size_t activeActorCount = GetActiveCount();

		for (size_t i = 0; i < activeActorCount; ++i)
		{
			if (m_actors[i] == actor)
			{
				m_positions[i] = position;
				return;
			}
		}
	}

	virtual void UpdateProxy(CAIActor* actor) override
	//Виртуальный метод обновиПрокси, принимает ук на АктораИИ
	{
		size_t activeActorCount = GetActiveCount();

		for (size_t i = 0; i < activeActorCount; ++i)
		{
			if (m_actors[i] == actor)
			{
				m_proxies[i] = actor->GetProxy();
				return;
			}
		}
	}

	virtual void Prepare(u32 lookUpFields) override
	//Виртуальный метод подготовь, принимает бцел32 поляПоиска
	{
		if (!m_actors.empty())
		{
			DrxPrefetch(&m_actors[0]);

			if (lookUpFields & Proxy)
			{
				DrxPrefetch(&m_proxies[0]);
			}

			if (lookUpFields & Position)
			{
				DrxPrefetch(&m_positions[0]);
			}

			if (lookUpFields & EntityID)
			{
				DrxPrefetch(&m_entityIDs[0]);
			}
		}
	}

	virtual void AddActor(CAIActor* actor) override
	//Виртуальный метод добавьАктора, принимает ук на АктораИИ
	{
		assert(actor);

		size_t activeActorCount = GetActiveCount();

		for (u32 i = 0; i < activeActorCount; ++i)
		{
			if (m_actors[i] == actor)
			{
				m_proxies[i] = actor->GetProxy();
				m_positions[i] = actor->GetPos();
				m_entityIDs[i] = actor->GetEntityID();

				return;
			}
		}

		m_actors.push_back(actor);
		m_proxies.push_back(actor->GetProxy());
		m_positions.push_back(actor->GetPos());
		m_entityIDs.push_back(actor->GetEntityID());
	}

	virtual void RemoveActor(CAIActor* actor) override
	//Виртуальный метод удалиАктора, принимает ук на АктораИИ
	{
		assert(actor);

		size_t activeActorCount = GetActiveCount();

		for (size_t i = 0; i < activeActorCount; ++i)
		{
			if (m_actors[i] == actor)
			{
				std::swap(m_actors[i], m_actors.back());
				m_actors.pop_back();

				std::swap(m_proxies[i], m_proxies.back());
				m_proxies.pop_back();

				std::swap(m_positions[i], m_positions.back());
				m_positions.pop_back();

				std::swap(m_entityIDs[i], m_entityIDs.back());
				m_entityIDs.pop_back();

				return;
			}
		}
	}

private:
	virtual IAIActor* GetActorInternal(u32 index) const override
	{
		if (index >= m_actors.size())
			return nullptr;

		return m_actors[index];
	}
//Приватные Векторы
	typedef std::vector<CAIActor*> Actors;
//Акторы
	Actors m_actors;

	typedef std::vector<IAIActorProxy*> Proxies;
//Прокси
	Proxies m_proxies;

	typedef std::vector<Vec3> Positions;
//Позиции
	Positions m_positions;

	typedef std::vector<EntityId> EntityIDs;
//ИдыСущностей
	EntityIDs m_entityIDs;
};

#endif
