// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a part for vehicles which uses CGA files

   -------------------------------------------------------------------------
   История:
   - 24:08:2005: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEPARTANIMATED_H__
#define __VEHICLEPARTANIMATED_H__

#include "VehiclePartBase.h"

class CScriptBind_VehiclePart;
class CVehicle;
class CVehiclePartAnimatedJoint;

class CVehiclePartAnimated
	: public CVehiclePartBase
{
	IMPLEMENT_VEHICLEOBJECT
public:

	CVehiclePartAnimated();
	~CVehiclePartAnimated();

	// IVehiclePart
	virtual bool Init(IVehicle* pVehicle, const CVehicleParams& table, IVehiclePart* parent, CVehicle::SPartInitInfo& initInfo, i32 partType) override;
	virtual void Release() override;
	virtual void Reset() override;

	virtual void SetMaterial(IMaterial* pMaterial) override;

	virtual void OnEvent(const SVehiclePartEvent& event) override;

	virtual bool ChangeState(EVehiclePartState state, i32 flags = 0) override;
	virtual void Physicalize() override;

	virtual void Update(const float frameTime) override;

	virtual void Serialize(TSerialize serialize, EEntityAspects) override;
	virtual void PostSerialize() override;

	virtual void GetMemoryUsage(IDrxSizer* s) const override;
	// ~IVehiclePart

	virtual IStatObj* GetSubGeometry(CVehiclePartBase* pPart, EVehiclePartState state, Matrix34& position, bool removeFromParent) override;
	IStatObj*         GetGeometryForState(CVehiclePartAnimatedJoint* pPart, EVehiclePartState state);
	IStatObj*         GetDestroyedGeometry(tukk pJointName, u32 index = 0);
	Matrix34          GetDestroyedGeometryTM(tukk pJointName, u32 index);

	virtual void      SetDrivingProxy(bool bDrive);

	void              RotationChanged(CVehiclePartAnimatedJoint* pJoint);

	i32               AssignAnimationLayer() { return ++m_lastAnimLayerAssigned; }

#if ENABLE_VEHICLE_DEBUG
	void Dump();
#endif

	i32  GetNextFreeLayer();
	bool ChangeChildState(CVehiclePartAnimatedJoint* pPart, EVehiclePartState state, i32 flags);

protected:

	virtual void              InitGeometry();
	void                      FlagSkeleton(ISkeletonPose* pSkeletonPose, IDefaultSkeleton& rIDefaultSkeleton);
	virtual EVehiclePartState GetStateForDamageRatio(float ratio) override;

	typedef std::map<string, /*_smart_ptr<*/ IStatObj*> TStringStatObjMap;
	TStringStatObjMap m_intactStatObjs;

	typedef std::map<string, IVehiclePart*> TStringVehiclePartMap;
	TStringVehiclePartMap          m_jointParts;

	_smart_ptr<ICharacterInstance> m_pCharInstance;
	ICharacterInstance*            m_pCharInstanceDestroyed;

	i32                            m_hullMatId[2];

	i32                            m_lastAnimLayerAssigned;
	i32                            m_iRotChangedFrameId;
	bool                           m_serializeForceRotationUpdate;
	bool                           m_initialiseOnChangeState;
	bool                           m_ignoreDestroyedState;
};

#endif
