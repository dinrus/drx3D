// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Vehicle System

   -------------------------------------------------------------------------
   История:
   - 05:10:2004   11:55 : Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLESYSTEM_H__
#define __VEHICLESYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include "IVehicleSystem.h"

typedef std::map<string, IGameObjectExtensionCreatorBase*> TVehicleClassMap;
typedef std::map<EntityId, IVehicle*>                      TVehicleMap;
typedef std::map<string, IVehicleMovement*(*)()>           TVehicleMovementClassMap;
typedef std::map<string, IVehicleView*(*)()>               TVehicleViewClassMap;
typedef std::map<string, IVehiclePart*(*)()>               TVehiclePartClassMap;
typedef std::map<string, IVehicleSeatAction*(*)()>         TVehicleSeatActionClassMap;
typedef std::map<string, IVehicleAction*(*)()>             TVehicleActionClassMap;
typedef std::vector<IVehicleUsageEventListener*>           TVehicleUsageListenerVec;
typedef std::map<EntityId, TVehicleUsageListenerVec>       TVehicleUsageEventListenerList;

#if ENABLE_VEHICLE_DEBUG
typedef DrxFixedStringT<32> TVehicleListenerId;
#else
typedef i32                 TVehicleListenerId;
#endif

class CVehicleCVars;
class CVehicleSeat;

class CVehicleSystem :
	public IVehicleSystem
{
public:
	CVehicleSystem(ISystem* pSystem, IEntitySystem* pEntitySystem);
	virtual ~CVehicleSystem();

	virtual void Release() { delete this; };

	virtual void Reset();

	virtual void RegisterFactory(tukk name, IVehicleMovement*(*func)(), bool isAI)       { RegisterVehicleMovement(name, func, isAI); };
	virtual void RegisterFactory(tukk name, IVehicleView*(*func)(), bool isAI)           { RegisterVehicleView(name, func, isAI); }
	virtual void RegisterFactory(tukk name, IVehiclePart*(*func)(), bool isAI)           { RegisterVehiclePart(name, func, isAI); }
	virtual void RegisterFactory(tukk name, IVehicleDamageBehavior*(*func)(), bool isAI) { RegisterVehicleDamageBehavior(name, func, isAI); }
	virtual void RegisterFactory(tukk name, IVehicleSeatAction*(*func)(), bool isAI)     { RegisterVehicleSeatAction(name, func, isAI); }
	virtual void RegisterFactory(tukk name, IVehicleAction*(*func)(), bool isAI)         { RegisterVehicleAction(name, func, isAI); }

	// IVehicleSystem
	virtual bool                             Init();
	virtual IVehicle*                        CreateVehicle(u16 channelId, tukk name, tukk vehicleClass, const Vec3& pos, const Quat& rot, const Vec3& scale, EntityId id = 0);
	virtual IVehicle*                        GetVehicle(EntityId entityId);
	virtual IVehicle*                        GetVehicleByChannelId(u16 channelId);
	virtual bool                             IsVehicleClass(tukk name) const;

	virtual IVehicleMovement*                CreateVehicleMovement(const string& name);
	virtual IVehicleView*                    CreateVehicleView(const string& name);
	virtual IVehiclePart*                    CreateVehiclePart(const string& name);
	virtual IVehicleDamageBehavior*          CreateVehicleDamageBehavior(const string& name);
	virtual IVehicleSeatAction*              CreateVehicleSeatAction(const string& name);
	virtual IVehicleAction*                  CreateVehicleAction(const string& name);

	virtual void                             RegisterVehicles(IGameFramework* gameFramework);

	virtual IVehicleDamagesTemplateRegistry* GetDamagesTemplateRegistry() { return m_pDamagesTemplateRegistry; }

	virtual TVehicleObjectId                 AssignVehicleObjectId();
	virtual TVehicleObjectId                 AssignVehicleObjectId(const string& className);
	virtual TVehicleObjectId                 GetVehicleObjectId(const string& className) const;

	virtual u32                           GetVehicleCount() { return u32(m_vehicles.size()); }
	virtual IVehicleIteratorPtr              CreateVehicleIterator();

	virtual IVehicleClient*                  GetVehicleClient() { return m_pVehicleClient; }
	virtual void                             RegisterVehicleClient(IVehicleClient* pVehicleClient);

	virtual void                             RegisterVehicleUsageEventListener(const EntityId playerId, IVehicleUsageEventListener* pEventListener);
	virtual void                             UnregisterVehicleUsageEventListener(const EntityId playerId, IVehicleUsageEventListener* pEventListener);
	virtual void                             BroadcastVehicleUsageEvent(const EVehicleEvent eventId, const EntityId playerId, IVehicle* pVehicle);

	virtual void                             Update(float deltaTime);
	// ~IVehicleSystem

	void                SetInitializingSeat(CVehicleSeat* pSeat) { m_pInitializingSeat = pSeat; };
	CVehicleSeat*       GetInitializingSeat()                    { return m_pInitializingSeat; };

	void                RegisterVehicleClass(tukk name, IGameFramework::IVehicleCreator* pCreator, bool isAI);
	void                AddVehicle(EntityId entityId, IVehicle* pProxy);
	void                RemoveVehicle(EntityId entityId);

	void                GetVehicleImplementations(SVehicleImpls& impls);
	bool                GetOptionalScript(tukk vehicleName, tuk buf, size_t len);

	void                RegisterVehicleMovement(tukk name, IVehicleMovement*(*func)(), bool isAI);
	void                RegisterVehicleView(tukk name, IVehicleView*(*func)(), bool isAI);
	void                RegisterVehiclePart(tukk name, IVehiclePart*(*func)(), bool isAI);
	void                RegisterVehicleDamageBehavior(tukk name, IVehicleDamageBehavior*(*func)(), bool isAI);
	void                RegisterVehicleSeatAction(tukk name, IVehicleSeatAction*(*func)(), bool isAI);
	void                RegisterVehicleAction(tukk name, IVehicleAction*(*func)(), bool isAI);

	void                RegisterVehicleObject(const string& className, TVehicleObjectId id);

	void                LoadDamageTemplates();
	void                ReloadSystem();

	bool                OnStartUse(const EntityId playerId, IVehicle* pVehicle);
	void                OnPrePhysicsTimeStep(float deltaTime);

	DrxCriticalSection& GetCurrentVehicleLock()     { return m_currentVehicleLock; }
	 IVehicle*  GetCurrentClientVehicle()   { return m_pCurrentClientVehicle; }
	void                ClearCurrentClientVehicle() { m_pCurrentClientVehicle = NULL; }
	void                GetMemoryStatistics(IDrxSizer* s);

#if ENABLE_VEHICLE_DEBUG
	static void DumpClasses(IConsoleCmdArgs* pArgs);

	typedef std::map<string, i32> TVehicleClassCount;
	static TVehicleClassCount s_classInstanceCounts;
#endif

private:
	//	static IEntityComponent *CreateVehicle(IEntity *pEntity, SEntitySpawnParams &params, uk pUserData);
	void RegisterCVars();

	struct SSpawnUserData
	{
		SSpawnUserData(tukk cls, u16 channel) : className(cls), channelId(channel) {}
		tukk className;
		u16      channelId;
	};

	struct SVehicleUserData
	{
		SVehicleUserData(tukk cls, CVehicleSystem* pNewVehicleSystem) : className(cls), pVehicleSystem(pNewVehicleSystem) {};
		CVehicleSystem* pVehicleSystem;
		string          className;
	};

	class CVehicleIterator : public IVehicleIterator
	{
	public:
		CVehicleIterator(CVehicleSystem* pVS)
		{
			m_pVS = pVS;
			m_cur = m_pVS->m_vehicles.begin();
			m_nRefs = 0;
		}
		void AddRef()
		{
			++m_nRefs;
		}
		void Release()
		{
			if (--m_nRefs <= 0)
			{
				assert(std::find(m_pVS->m_iteratorPool.begin(),
				                 m_pVS->m_iteratorPool.end(), this) == m_pVS->m_iteratorPool.end());
				// Call my own destructor before I push to the pool - avoids tripping up the STLP debugging {2008/12/09})
				this->~CVehicleIterator();
				m_pVS->m_iteratorPool.push_back(this);
			}
		}
		IVehicle* Next()
		{
			if (m_cur == m_pVS->m_vehicles.end())
				return 0;
			IVehicle* pVehicle = m_cur->second;
			++m_cur;
			return pVehicle;
		}
		size_t Count()
		{
			return m_pVS->m_vehicles.size();
		}
		CVehicleSystem*       m_pVS;
		TVehicleMap::iterator m_cur;
		i32                   m_nRefs;
	};

	TVehicleMap                m_vehicles;
	TVehicleClassMap           m_classes;
	TVehicleMovementClassMap   m_movementClasses;
	TVehicleViewClassMap       m_viewClasses;
	TVehiclePartClassMap       m_partClasses;
	TVehicleSeatActionClassMap m_seatActionClasses;
	TVehicleActionClassMap     m_actionClasses;

	typedef std::map<string, IVehicleDamageBehavior*(*)()> TVehicleDamageBehaviorClassMap;
	TVehicleDamageBehaviorClassMap m_damageBehaviorClasses;

	typedef std::map<string, TVehicleObjectId> TVehicleObjectIdMap;
	TVehicleObjectIdMap              m_objectIds;
	TVehicleObjectId                 m_nextObjectId;

	IVehicleDamagesTemplateRegistry* m_pDamagesTemplateRegistry;

	std::vector<CVehicleIterator*>   m_iteratorPool;

	IVehicleClient*                  m_pVehicleClient;
	CVehicleCVars*                   m_pCVars;

	CVehicleSeat*                    m_pInitializingSeat;
	 IVehicle*               m_pCurrentClientVehicle; // Pointer to the vehicle the client is currently in

	TVehicleUsageEventListenerList   m_eventListeners;

	DrxCriticalSection               m_currentVehicleLock;
};

// Summary:
//  implements SEnvironmentLayer
class CEnvironmentLayer : public SEnvironmentLayer
{
public:
	CEnvironmentLayer()
	{
	}
	virtual ~CEnvironmentLayer(){}

	tukk GetName() const { return name.c_str(); }

	size_t      GetHelperCount() const
	{
		return helpers.size();
	}

	IVehicleHelper* GetHelper(i32 idx) const
	{
		DRX_ASSERT(idx >= 0 && idx < (i32)helpers.size());
		return helpers[idx];
	}

	size_t GetGroupCount() const
	{
		return wheelGroups.size();
	}

	size_t GetWheelCount(i32 group) const
	{
		assert(group >= 0);

		if (group < (i32)wheelGroups.size())
			return wheelGroups[group].m_wheels.size();

		return 0;
	}

	i32 GetWheelAt(i32 group, i32 wheel) const
	{
		assert(group >= 0);
		assert(wheel >= 0);

		if (group < (i32)wheelGroups.size() && wheel < (i32)wheelGroups[group].m_wheels.size())
			return wheelGroups[group].m_wheels[wheel];

		assert(0 && "GetWheelAt: idx out of bounds");
		return 0;
	}

	bool IsGroupActive(i32 group) const
	{
		assert(group >= 0);
		if (group < (i32)wheelGroups.size())
			return wheelGroups[group].active;

		return false;
	}

	void SetGroupActive(i32 group, bool active)
	{
		if (group < (i32)wheelGroups.size())
			wheelGroups[group].active = active;
	}

protected:

	struct SWheelGroup
	{
		std::vector<i32> m_wheels;
		bool             active;

		SWheelGroup()
		{
			active = true;
		}
	};

	string                       name;
	std::vector<SWheelGroup>     wheelGroups;
	std::vector<IVehicleHelper*> helpers;

	friend class CVehicle;
};

// Summary:
// implements SEnvironmentParticles
class CEnvironmentParticles : public SEnvironmentParticles
{
public:
	size_t GetLayerCount() const
	{
		return layers.size();
	}
	const SEnvironmentLayer& GetLayer(i32 idx) const
	{
		return layers[idx];
	}
	tukk GetMFXRowName() const
	{
		return mfx_rowName.c_str();
	}

protected:
	string                         mfx_rowName; // the row name to use to lookup effects in the material effects file (previously hardcoded to vfx_<vehicleclass> )
	std::vector<CEnvironmentLayer> layers;

	friend class CVehicle;
};

// Summary:
// implements SExhaustParams
class CExhaustParams : public SExhaustParams
{
public:
	CExhaustParams()
	{
	}
	size_t GetExhaustCount() const
	{
		return helpers.size();
	}
	IVehicleHelper* GetHelper(i32 idx) const
	{
		DRX_ASSERT(idx < (i32)helpers.size());
		return helpers[idx];
	}
	tukk GetStartEffect() const { return startEffect.c_str(); }
	tukk GetStopEffect() const  { return stopEffect.c_str(); }
	tukk GetRunEffect() const   { return runEffect.c_str(); }
	tukk GetBoostEffect() const { return boostEffect.c_str(); }

protected:
	string                       startEffect;
	string                       stopEffect;
	string                       runEffect;
	string                       boostEffect;

	std::vector<IVehicleHelper*> helpers; // one or more exhausts possible

	friend class CVehicle;
};

// Summary:
//  implements SParticleParams
class CParticleParams : public SParticleParams
{
public:
	SExhaustParams*      GetExhaustParams() { return &m_exhaustParams; }
	const SDamageEffect* GetDamageEffect(tukk pName) const
	{
		if (pName)
		{
			TDamageEffectMap::const_iterator iDamageEffect = damageEffects.find(pName);

			if (iDamageEffect != damageEffects.end())
			{
				return &iDamageEffect->second;
			}
		}

		return NULL;
	}

	SEnvironmentParticles* GetEnvironmentParticles() { return &m_envParams; }

protected:
	CExhaustParams        m_exhaustParams;
	TDamageEffectMap      damageEffects;
	CEnvironmentParticles m_envParams;

	friend class CVehicle;
};

struct pe_cargeomparams;

#endif //__VEHICLESYSTEM_H__
