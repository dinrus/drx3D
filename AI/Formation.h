// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/PathMarker.h>
#include <drx3D/Sys/TimeValue.h>
#include <vector>

struct FormationNode;
class CFormationDescriptor;

class CAIObject;
class CAIActor;
class CAISystem;

struct CFormationPoint
{
	CFormationPoint();
	~CFormationPoint();
	void Serialize(TSerialize ser);

	Vec3                   m_vPoint;                  //TVectorOfVectors m_vPoints;
	Vec3                   m_vSight;                  //TVectorOfVectors m_vSights;
	float                  m_FollowDistance;          //	std::vector<float> m_vFollowDistances;
	float                  m_FollowOffset;            //std::vector<float> m_vFollowOffsets;
	float                  m_FollowDistanceAlternate; //	std::vector<float> m_vFollowDistancesAlternate;
	float                  m_FollowOffsetAlternate;   //std::vector<float> m_vFollowOffsetsAlternate;
	float                  m_FollowHeightOffset;      //std::vector<float> m_vFollowOffsets;
	float                  m_Speed;                   //	std::vector<float> m_Speeds;
	CTimeValue             m_fLastUpdateSightTime;
	Vec3                   m_LastPosition;   // TVectorOfVectors m_vLastPosition;
	Vec3                   m_Dir;            //TVectorOfVectors m_vDir;
	i32                    m_Class;          //std::vector<i32> m_vClasses;
	CCountedRef<CAIObject> m_refWorldPoint;  //TFormationDummies m_vWorldPoints;
	CWeakRef<CAIActor>     m_refReservation; //TFormationDummies m_vReservations;
	CCountedRef<CAIObject> m_refDummyTarget; //TFormationDummies  m_vpDummyTargets;
	bool                   m_FollowFlag;     //std::vector<bool> m_vFollowFlags;
	Vec3                   m_DEBUG_vInitPos; // Initial position for debugging.

	void SetPos(Vec3 pos, const Vec3& startPos, bool force = false, bool bPlayerLeader = false);
};

typedef std::vector<CFormationPoint> TFormationPoints;

class CFormation
{
public:
	enum eOrientationType
	{
		OT_MOVE,
		OT_VIEW
	};

	typedef i32 TFormationID;
	static const TFormationID INVALID_FORMATION_ID = -1;
	static TFormationID       s_NextFormation;

private:
	TFormationID     m_formationID;
	TFormationPoints m_FormationPoints;

	//	Vec3 m_vPos;
	//	Vec3 m_vAngles;

	bool                      m_bReservationAllowed;

	Vec3                      m_vLastUpdatePos;
	Vec3                      m_vLastPos;
	Vec3                      m_vLastTargetPos;
	Vec3                      m_vLastMoveDir;
	Vec3                      m_vInitialDir;
	Vec3                      m_vBodyDir;
	Vec3                      m_vMoveDir;
	bool                      m_bInitMovement;
	bool                      m_bFirstUpdate;
	bool                      m_bUpdate;
	bool                      m_bForceReachable;
	bool                      m_bFollowingUnits;
	bool                      m_bFollowPointInitialized;
	float                     m_fUpdateThreshold;
	float                     m_fDynamicUpdateThreshold;
	float                     m_fStaticUpdateThreshold;
	float                     m_fScaleFactor;
	static const float        m_fDISTANCE_UPDATE_THR;
	static const float        m_fSIGHT_WIDTH;
	CPathMarker*              m_pPathMarker;
	i32                       m_iSpecialPointIndex;
	float                     m_fMaxFollowDistance;
	CTimeValue                m_fLastUpdateTime;
	i32                       m_maxUpdateSightTimeMs;
	i32                       m_minUpdateSightTimeMs;
	float                     m_fSightRotationRange;
	eOrientationType          m_orientationType;

	Vec3                      m_dbgVector1;
	Vec3                      m_dbgVector2;
	string                    m_szDescriptor;
	CWeakRef<const CAIObject> m_refReferenceTarget;
	CWeakRef<CAIObject>       m_refOwner;                   // (MATT) Could be a pipeuser or such? {2009/03/18}
	float                     m_fRepositionSpeedWrefTarget;
	Vec3                      m_vDesiredTargetPos;

public:
	CFormation();
	~CFormation(void);
	void         ReleasePoints();
	void         OnObjectRemoved(CAIObject* pObject);
	// fills the formation class with all necessary information and set the points in the world
	void         Create(CFormationDescriptor& desc, CWeakRef<CAIObject> refOwner, Vec3 vTargetPos);
	// Update of the formation (refreshes position of formation points)
	void         Update(/*CAIObject *pOwner */);
	// Changes the position of formation points
	void         Change(CFormationDescriptor& desc, float fScale);

	float        GetMaxWidth();
	TFormationID GetId() { return m_formationID; }

	// Init the world positions of the formation points
	void           InitWorldPosition(Vec3 dir = ZERO);
	// returns an available formation point, if that exists
	CAIObject*     GetNewFormationPoint(CWeakRef<CAIActor> refRequester, i32 index = -1);
	i32            GetClosestPointIndex(CWeakRef<CAIActor> refRequester, bool bReserve = false, i32 maxSize = 0, i32 iClass = -1, bool bClosestToOwner = false);
	CAIObject*     GetClosestPoint(CAIActor* pRequester, bool bReserve = false, i32 iClass = -1);
	CAIObject*     GetFormationDummyTarget(CWeakRef<const CAIObject> refRequester) const;
	inline i32     GetSize() const { return static_cast<i32>(m_FormationPoints.size()); }
	inline Vec3    GetBodyDir()    { return m_vBodyDir;  }
	inline Vec3    GetMoveDir()    { return m_vMoveDir;  }
	inline string& GetDescriptor() { return m_szDescriptor; }
	void           SetScale(float scale);

	// (MATT) This isn't actually called anywhere. Could surely go. {2009/03/23}
	void                    SetReferenceTarget(const CAIObject* pTarget, float repositionSpeed = 0);

	inline const CAIObject* GetReferenceTarget() const { return m_refReferenceTarget.GetAIObject(); }
	// if update=false, formation is not updated - it stays where it was last time
	void                    SetUpdate(bool bUpdate);
	inline void             ForceReachablePoints(bool force) { m_bForceReachable = force; }
	inline bool             IsUpdated() const                { return m_bUpdate; }

	inline bool             IsPointFree(i32 i) const         { return static_cast<bool>(m_FormationPoints[i].m_refReservation.GetAIObject() == NULL); }
	inline CAIActor*        GetPointOwner(i32 i) const       { return m_FormationPoints[i].m_refReservation.GetAIObject(); }
	void                    Draw();
	void                    FreeFormationPoint(CWeakRef<const CAIObject> refCurrentHolder);
	void                    FreeFormationPoint(i32 i);
	i32                     CountFreePoints() const;
	CAIObject*              GetFormationPoint(CWeakRef<const CAIObject> refRequester) const;
	inline CAIObject*       GetFormationPoint(i32 index) const { return (index >= 0 && index < (i32)m_FormationPoints.size() ? m_FormationPoints[index].m_refWorldPoint.GetAIObject() : NULL); };
	float                   GetFormationPointSpeed(CWeakRef<const CAIObject> refRequester) const;
	float                   GetDistanceToOwner(i32 i) const;
	inline i32              GetPointClass(i32 i) const { return m_FormationPoints[i].m_Class; }
	//	inline Vec3 GetPointPosition(i32 i) {return m_vWorldPoints[i];}
	void                    Reset();
	void                    Serialize(TSerialize ser);
	Vec3                    GetPredictedPointPosition(CWeakRef<const CAIObject> refRequestor, const Vec3& ownerPos, const Vec3& ownerLookDir, Vec3 ownerMoveDir) const;
	void                    SetUpdateSight(float angleRange, float minTime = 0.0f, float maxTime = 0.0f);

	inline void             SetOrientationType(eOrientationType t) { m_orientationType = t; }

	bool                    GetPointOffset(i32 i, Vec3& ret);

	const CPathMarker*      GetPathMarker() const { return m_pPathMarker; }
	CAIObject*              GetOwner();
	i32                     GetPointIndex(CWeakRef<const CAIObject> refRequester) const; // returns -1 if the requester has not acquired a slot in this formation

private:
	//void UpdateOrientations();
	void SetDummyTargetPosition(const Vec3& vPosition, CAIObject* pDummyTarget, const Vec3& vSight);
	void InitFollowPoints(const Vec3& vDir = ZERO);//, const CAIObject* pRequestor);
	void SetOffsetPointsPos(const Vec3& vMovedir);
	void SetUpdateThresholds(float value);
	Vec3 GetPointSmooth(const Vec3& headPos, float backDisr, float sideDist, float heightValue, i32 smoothValue, Vec3& pPointAlongCurve, CAIObject* pUser = NULL);
	void AddPoint(FormationNode& desc);
};

