// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef wheeledvehicleentity_h
#define wheeledvehicleentity_h
#pragma once

struct suspension_point {
	Vec3 pos;
	quaternionf q;
	float scale;
	Vec3 BBox[2];

	float Tscale;
	Vec3 pt; // the uppermost suspension point in car frame
	float fullen; // unconstrained length
	float kStiffness, kStiffnessWeight; // stiffness coefficient
	float kDamping,kDamping0; // damping coefficient
	float len0; // initial length in model
	float Mpt; // hull "mass" at suspension upper point along suspension direction
	quaternionf q0;	// used to calculate geometry transformation from wheel transformation
	Vec3 pos0,ptc0; // ...
	float Iinv;
	float minFriction,maxFriction;
  float kLatFriction;
	i32 flags0,flagsCollider0;
	u32 bDriving    : 1; // if the corresponding wheel a driving wheel
	u32 bCanBrake   : 1;
	u32 bBlocked    : 1;
	u32 bRayCast    : 1;
	u32 bSlip       : 1;
	u32 bSlipPull   : 1;
	u32 bContact    : 1;
	u32 bCanSteer   : 1;
	int8 iAxle;
	int8 iBuddy;
	i16 ipart;
	float r,rinv; // wheel radius, 1.0/radius
	float width;

	float curlen; // current length
	float steer; // steering angle
	float rot; // current wheel rotation angle
	float w; // current rotation speed
	float wa; // current angular acceleration
	float T; // wheel's net torque
	float prevTdt;
	float prevw;
	EventPhysCollision *pCollEvent;

	Vec3 ncontact,ptcontact; // filled in RegisterPendingCollisions
	i16 surface_idx[2];
	Vec3 vrel;
	Vec3 rworld;
	float vworld;
	float PN;
	RigidBody *pbody;
	CPhysicalEntity *pent;
	float unproj;
	entity_contact contact;
};

struct SWheeledVehicleEntityNetSerialize
{
	strided_pointer<suspension_point> pSusp;
	float pedal;
	float steer;
	float clutch;
	float wengine;
	bool handBrake;
	i32 curGear;

	void Serialize( TSerialize ser, i32 nSusp );
};

class CWheeledVehicleEntity : public CRigidEntity {
 public:
	explicit CWheeledVehicleEntity(CPhysicalWorld *pworld, IGeneralMemoryHeap* pHeap = NULL);
	virtual ~CWheeledVehicleEntity() { delete[] m_susp; }
	virtual pe_type GetType() const { return PE_WHEELEDVEHICLE; }

	virtual i32 SetParams(pe_params*,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params*) const;
	virtual i32 Action(pe_action*,i32 bThreadSafe=1);
	virtual i32 GetStatus(pe_status*) const;

	enum snapver { SNAPSHOT_VERSION = 1 };
	virtual i32 GetSnapshotVersion() { return SNAPSHOT_VERSION; }
	virtual i32 GetStateSnapshot(class CStream &stm, float time_back=0, i32 flags=0);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags=0);

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);

	virtual float GetMaxTimeStep(float time_interval);
	virtual float GetDamping(float time_interval);
	virtual float CalcEnergy(float time_interval);
	virtual void CheckAdditionalGeometry(float time_interval);
	virtual i32 RegisterContacts(float time_interval,i32 nMaxPlaneContacts);
	virtual i32 RemoveCollider(CPhysicalEntity *pCollider, bool bRemoveAlways=true);
	virtual i32 HasContactsWith(CPhysicalEntity *pent);
	virtual i32 HasPartContactsWith(CPhysicalEntity *pent, i32 ipart, i32 bGreaterOrEqual=0);
	virtual void AddAdditionalImpulses(float time_interval);
	virtual void AlertNeighbourhoodND(i32 mode);
	virtual i32 Update(float time_interval, float damping);
	virtual void ComputeBBox(Vec3 *BBox, i32 flags);
	//virtual RigidBody *GetRigidBody(i32 ipart=-1) { return &m_bodyStatic; }
	virtual void OnContactResolved(entity_contact *pcontact, i32 iop, i32 iGroupId);
	virtual void DrawHelperInformation(IPhysRenderer *pRenderer, i32 flags);

	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const;

	void RecalcSuspStiffness();
	float ComputeDrivingTorque(float time_interval);

	suspension_point *m_susp = nullptr;
	i32 m_suspAlloc = 0;
	float m_enginePower,m_maxSteer;
	float m_engineMaxw,m_engineMinw,m_engineIdlew,m_engineShiftUpw,m_engineShiftDownw,m_gearDirSwitchw,m_engineStartw;
	float m_axleFriction,m_brakeTorque,m_clutchSpeed,m_minBrakingFriction,m_maxBrakingFriction,m_kDynFriction,m_slipThreshold;
	float m_kStabilizer;
	float m_enginePedal,m_steer,m_ackermanOffset,m_clutch,m_wengine;
	float m_gears[12];
	int8 m_nGears,m_iCurGear;
	int8 m_maxGear,m_minGear;
	int8 m_bHandBrake;
	int8 m_nHullParts;
	int8 m_bHasContacts;
	// int8 m_iIntegrationType;
	int8 m_bKeepTractionWhenTilted;
	float m_kSteerToTrack;
	float m_EminRigid,m_EminVehicle;
	float m_maxAllowedStepVehicle,m_maxAllowedStepRigid;
	float m_dampingVehicle;
	//Vec3 m_Ffriction,m_Tfriction;
	float m_timeNoContacts;
	mutable  i32 m_lockVehicle;
  float m_pullTilt;
  float m_drivingTorque;
	float m_maxTilt;
	float m_wheelMassScale;
};

#endif
