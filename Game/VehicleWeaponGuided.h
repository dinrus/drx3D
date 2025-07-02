// Разработка 2018-2025 DinrusPro / Dinrus Group. ���� ������.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: VehicleWeapon Implementation

-------------------------------------------------------------------------
История:
- 20:04:2006   13:01 : Created by M�rcio Martins

*************************************************************************/
#if !defined(__VEHICLE_WEAPON_GUIDED_H__)
#define __VEHICLE_WEAPON_GUIDED_H__


#include <drx3D/Game/VehicleWeapon.h>

struct IVehicle;
struct IVehiclePart;
struct IVehicleSeat;
struct IVehicleAnimation;

class CVehicleWeaponGuided : public CVehicleWeapon
{
public:

	CVehicleWeaponGuided();
	virtual ~CVehicleWeaponGuided() {};
  
	// CWeapon
	virtual void ReadProperties(IScriptTable *pScriptTable);
	virtual bool Init(IGameObject * pGameObject);
	virtual void PostInit(IGameObject * pGameObject);
	virtual void Reset();

	virtual void StartFire();

	virtual void SetDestination(const Vec3& pos);
	virtual const Vec3& GetDestination();

	virtual void Update(SEntityUpdateContext& ctx, i32 update);

	//Vec3 GetSlotHelperPos(i32 slot, tukk helper, bool worldSpace, bool relative) const;

protected:

	enum eWeaponGuidedState
	{
		eWGS_INVALID,
		eWGS_PREPARATION,
		eWGS_WAIT,
		eWGS_FIRE,
		eWGS_FIRING,
		eWGS_POSTSTATE,
	};

	eWeaponGuidedState	m_State;
	eWeaponGuidedState	m_NextState;

	Vec3				m_DesiredHomingTarget;
	
	string				m_VehicleAnimName;
	string				m_PreStateName;
	string				m_PostStateName;

	float					m_firedTimer;
	
	IVehicleAnimation	*m_pVehicleAnim;
	i32					m_PreState;
	i32					m_PostState;
};



#endif //#if !defined(__VEHICLE_WEAPON_GUIDED_H__)