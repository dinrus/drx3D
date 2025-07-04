// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   birdsflock.h
//  Version:     v1.00
//  Created:     7/2010 by Luciano Morpurgo (refactored from Flock.h)
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual C++ 7.0
//  Описание: Specialized flocks for birds 
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __birdsflock_h__
#define __birdsflock_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include <drx3D/Game/Flock.h>
#include <drx3D/Game/BirdEnum.h>

struct SLandPoint : Vec3
{
	bool bTaken;
	SLandPoint(const Vec3& p)
	{
		x = p.x;
		y = p.y;
		z = p.z;
		bTaken = false;
	}
};

//////////////////////////////////////////////////////////////////////////
// Specialized flocks for birds 
//////////////////////////////////////////////////////////////////////////
class CBirdsFlock : public CFlock
{

public:
	CBirdsFlock( IEntity *pEntity );
	virtual void CreateBoids( SBoidsCreateContext &ctx );
	virtual void SetEnabled( bool bEnabled );
	virtual void Update( CCamera *pCamera );
	virtual void OnAIEvent(EAIStimulusType type, const Vec3& pos, float radius, float threat, EntityId sender);
	virtual void Reset();

	void SetAttractionPoint(const Vec3 &point);
	bool GetAttractOutput() const { return m_bAttractOutput; }
	void SetAttractOutput(bool bAO) { m_bAttractOutput = bAO; }
	void Land();
	void TakeOff();
	void NotifyBirdLanded();
	Vec3 GetLandingPoint(Vec3& fromPos);
	void LeaveLandingPoint(Vec3& point);
	inline bool IsPlayerNearOrigin() const {return m_isPlayerNearOrigin;}

protected:
	Vec3& FindLandSpot();
// 	void UpdateAvgBoidPos(float dt);
	float GetFlightDuration();
	i32 GetNumAliveBirds();
	void UpdateLandingPoints();
	void LandCollisionCallback(const QueuedRayID& rayID, const RayCastResult& result);
	bool IsPlayerInProximity(const Vec3& pos) const;

private:

	typedef std::vector<SLandPoint> TLandingPoints;

 	bool m_bAttractOutput;	// Set to true when the AttractTo flownode output has been activated
 	Vec3 m_defaultLandSpot;
	CTimeValue m_flightStartTime;
	Bird::EStatus m_status;
	float m_flightDuration;
	i32 m_birdsOnGround;
	TLandingPoints m_LandingPoints;
	i32 m_terrainPoints;
	bool 	m_hasLanded;
	bool	m_isPlayerNearOrigin;
	CBoidCollision m_landCollisionInfo;
	CGame::GlobalRayCaster::ResultCallback m_landCollisionCallback;
};


#endif // __flock_h__
