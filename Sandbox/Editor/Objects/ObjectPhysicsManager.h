// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ObjectPhysicsManager_h__
#define __ObjectPhysicsManager_h__

#if _MSC_VER > 1000
	#pragma once
#endif

//////////////////////////////////////////////////////////////////////////
class CObjectPhysicsManager
{
public:
	CObjectPhysicsManager();
	~CObjectPhysicsManager();

	void SimulateSelectedObjectsPositions();
	void Update();

	//////////////////////////////////////////////////////////////////////////
	/// Collision Classes
	//////////////////////////////////////////////////////////////////////////
	i32  RegisterCollisionClass(const SCollisionClass& collclass);
	i32  GetCollisionClassId(const SCollisionClass& collclass);
	void SerializeCollisionClasses(CXmlArchive& xmlAr);

	void PrepareForExport();

	void Command_SimulateObjects();
	void Command_GetPhysicsState();
	void Command_ResetPhysicsState();
	void Command_GenerateJoints();

private:

	void UpdateSimulatingObjects();

	bool                                 m_bSimulatingObjects;
	float                                m_fStartObjectSimulationTime;
	i32                                  m_wasSimObjects;
	std::vector<_smart_ptr<CBaseObject>> m_simObjects;
	CWaitProgress*                       m_pProgress;

	typedef std::vector<SCollisionClass> TCollisionClassVector;
	i32                   m_collisionClassExportId;
	TCollisionClassVector m_collisionClasses;
};

#endif // __ObjectPhysicsManager_h__

