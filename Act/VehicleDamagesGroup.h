// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements the base of the vehicle damages group

   -------------------------------------------------------------------------
   История:
   - 23:02:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEDAMAGESGROUP_H__
#define __VEHICLEDAMAGESGROUP_H__

class CVehicle;

class CVehicleDamagesGroup
	: public IVehicleDamagesGroup
{
public:

	virtual ~CVehicleDamagesGroup();

	bool          Init(CVehicle* pVehicle, const CVehicleParams& table);
	void          Release() { delete this; }
	void          Reset();
	void          Serialize(TSerialize ser, EEntityAspects aspects);

	virtual bool  ParseDamagesGroup(const CVehicleParams& table);

	const string& GetName() { return m_name; }
	bool          IsPotentiallyFatal();

	void          OnDamageEvent(EVehicleDamageBehaviorEvent event, const SVehicleDamageBehaviorEventParams& behaviorParams);
	void          Update(float frameTime);

	void          GetMemoryUsage(IDrxSizer* pSizer) const;

protected:

	typedef std::vector<IVehicleDamageBehavior*> TVehicleDamageBehaviorVector;

	typedef u16                               TDamagesSubGroupId;
	struct SDamagesSubGroup
	{
		TDamagesSubGroupId           id;
		float                        m_randomness;
		TVehicleDamageBehaviorVector m_damageBehaviors;
		float                        m_delay;
		bool                         m_isAlreadyInProcess;

		SDamagesSubGroup()
		{
			id = ~0;
			m_randomness = 0.f;
			m_delay = 0.f;
			m_isAlreadyInProcess = false;
		}
		void GetMemoryUsage(IDrxSizer* pSizer) const {}
	};

	typedef std::vector<SDamagesSubGroup> TDamagesSubGroupVector;
	TDamagesSubGroupVector m_damageSubGroups;

protected:

	IVehicleDamageBehavior* ParseDamageBehavior(const CVehicleParams& table);

	CVehicle* m_pVehicle;
	string    m_name;

	struct SDelayedDamagesSubGroupInfo
	{
		float                             delay;
		TDamagesSubGroupId                subGroupId;
		SVehicleDamageBehaviorEventParams behaviorParams;
	};

	typedef std::list<SDelayedDamagesSubGroupInfo> TDelayedDamagesSubGroupList;
	TDelayedDamagesSubGroupList m_delayedSubGroups;

	friend class CVehicleDamagesTemplateRegistry;
};

#endif
