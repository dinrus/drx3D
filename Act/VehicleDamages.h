// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements the base of the vehicle damages

   -------------------------------------------------------------------------
   История:
   - 23:02:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEDAMAGES_H__
#define __VEHICLEDAMAGES_H__

class CVehicle;
class CVehicleDamagesGroup;

class CVehicleDamages
{
public:

	CVehicleDamages() : m_pVehicle(NULL) {}

	struct SDamageMultiplier
	{
		float mult;
		float splash;

		SDamageMultiplier() : mult(1.f), splash(1.f) {}

		void GetMemoryUsage(IDrxSizer* pSizer) const { /*nothing*/ }
	};

	enum
	{
		DEFAULT_HIT_TYPE = 0
	};

	void                  InitDamages(CVehicle* pVehicle, const CVehicleParams& table);
	void                  ReleaseDamages();
	void                  ResetDamages();
	void                  UpdateDamages(float frameTime);

	void                  GetDamagesMemoryStatistics(IDrxSizer* pSizer) const;

	bool                  ProcessHit(float& damage, const HitInfo& hitInfo, bool splash);
	CVehicleDamagesGroup* GetDamagesGroup(tukk groupName);

	typedef std::map<i32, SDamageMultiplier> TDamageMultipliers;
	static void ParseDamageMultipliers(TDamageMultipliers& multipliersByHitType, TDamageMultipliers& multipliersByProjectile, const CVehicleParams& table, const IEntity& entity);

protected:
	CVehicle* m_pVehicle;

	typedef std::vector<CVehicleDamagesGroup*> TVehicleDamagesGroupVector;
	TVehicleDamagesGroupVector m_damagesGroups;

	SVehicleDamageParams       m_damageParams;

	TDamageMultipliers         m_damageMultipliersByHitType;
	TDamageMultipliers         m_damageMultipliersByProjectile;
};

#endif
