// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   PhysCallbacks.h
//  Created:     14/11/2006 by Anton.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PhysCallbacks_h__
#define __PhysCallbacks_h__
#pragma once

#include <drx3D/Phys/IDeferredCollisionEvent.h>

class CPhysCallbacks : public DinrusX3dEngBase
{
public:
	static void Init();
	static void Done();

	static i32  OnFoliageTouched(const EventPhys* pEvent);
	static i32  OnPhysStateChange(const EventPhys* pEvent);
	//static i32 OnPhysAreaChange(const EventPhys *pEvent);
};

class CPhysStreamer : public IPhysicsStreamer
{
public:
	virtual i32 CreatePhysicalEntity(uk pForeignData, i32 iForeignData, i32 iForeignFlags);
	virtual i32 DestroyPhysicalEntity(IPhysicalEntity* pent) { return 1; }
	virtual i32 CreatePhysicalEntitiesInBox(const Vec3& boxMin, const Vec3& boxMax);
	virtual i32 DestroyPhysicalEntitiesInBox(const Vec3& boxMin, const Vec3& boxMax);
};

// Deferred physics event object implementing CPhysCallbacks::OnCollision
class CDeferredCollisionEventOnPhysCollision : public IDeferredPhysicsEvent, public DinrusX3dEngBase
{
public:
	virtual ~CDeferredCollisionEventOnPhysCollision();

	// Factory create function to pass to CDeferredPhysicsUpr::HandleEvent
	static IDeferredPhysicsEvent* CreateCollisionEvent(const EventPhys* pCollisionEvent);

	// Entry function to register as event handler with physics
	static i32 OnCollision(const EventPhys* pEvent);

	// == IDeferredPhysicsEvent interface == //
	virtual void                                     Start();
	virtual void                                     Execute();
	virtual void                                     ExecuteAsJob();
	virtual i32                                      Result(EventPhys*);
	virtual void                                     Sync();
	virtual bool                                     HasFinished();

	virtual IDeferredPhysicsEvent::DeferredEventType GetType() const { return PhysCallBack_OnCollision; }
	virtual EventPhys*                               PhysicsEvent()  { return &m_CollisionEvent; }

	uk                                            operator new(size_t);
	void                                             operator delete(uk );

private:
	// Private constructor, only allow creating by the factory function(which is used by the deferred physics event manager
	CDeferredCollisionEventOnPhysCollision(const EventPhysCollision* pCollisionEvent);

	// == Functions implementing the event logic == //
	void RayTraceVegetation();
	void TestCollisionWithRenderMesh();
	void FinishTestCollisionWithRenderMesh();
	void PostStep();
	void AdjustBulletVelocity();
	void UpdateFoliage();

	// == utility functions == //
	void MarkFinished(i32 nResult);

	// == state variables to sync the asynchron execution == //
	JobUpr::SJobState m_jobStateRayInter;
	JobUpr::SJobState m_jobStateOnCollision;

	 bool         m_bTaskRunning;

	// == members for internal state of the event == //
	bool m_bHit;                              // did  the RayIntersection hit something
	bool m_bFinished;                         // did an early out happen and we are finished with computation
	bool m_bPierceable;                       // remember if we are hitting a pierceable object
	i32  m_nResult;                           // result value of the event(0 or 1)
	CDeferredCollisionEventOnPhysCollision* m_pNextEvent;
	i32  m_nWaitForPrevEvent;

	// == internal data == //
	EventPhysCollision      m_CollisionEvent;               // copy of the original physics event
	SRayHitInfo             m_HitInfo;                      // RayIntersection result data
	_smart_ptr<IRenderMesh> m_pRenderMesh;                  // Rendermesh to use for RayIntersection
	bool                    m_bDecalPlacementTestRequested; // check if decal can be placed here
	_smart_ptr<IMaterial>   m_pMaterial;                    // Material for IMaterial
	bool                    m_bNeedMeshThreadUnLock;        // remeber if we need to unlock a rendermesh

	// == members to store values over functions == //
	Matrix34 m_worldTM;
	Matrix33 m_worldRot;
	i32*     m_pMatMapping;
	i32      m_nMats;

	// == States for ThreadTask and AsyncRayIntersection == //
	SIntersectionData m_RayIntersectionData;

};

#endif // __PhysCallbacks_h__
