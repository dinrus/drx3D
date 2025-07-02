// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   -------------------------------------------------------------------------
   Имя файла:   GoalOp_Drxsis2.h
   Описание: Drxsi2 GoalOps
             These should move into GameDLL when interfaces allow!
   -------------------------------------------------------------------------
   История:
   - 18:11:2009 - Created by Márcio Martins
 *********************************************************************/

#ifndef __GoalOp_Drxsis2_H__
#define __GoalOp_Drxsis2_H__

#pragma once

#include <drx3D/AI/GoalOp.h>
#include <drx3D/AI/GoalOpFactory.h>
#include <drx3D/AI/Communication.h>
#include <drx3D/AI/PostureUpr.h>
#include <drx3D/AI/GenericAStarSolver.h>

// Forward declarations
class COPPathFind;
class COPTrace;

/**
 * Factory for Drxsis2 goalops
 *
 */
class CGoalOpFactoryDrxsis2 : public IGoalOpFactory
{
	IGoalOp* GetGoalOp(tukk sGoalOpName, IFunctionHandler* pH, i32 nFirstParam, GoalParameters& params) const;
	IGoalOp* GetGoalOp(EGoalOperations op, GoalParameters& params) const;
};

////////////////////////////////////////////////////////
//
//	Adjust aim while staying still.
//
////////////////////////////////////////////////////////
class COPDrxsis2AdjustAim : public CGoalOp
{
	CTimeValue                     m_startTime;
	CTimeValue                     m_lastGood;
	bool                           m_useLastOpAsBackup;
	bool                           m_allowProne;
	float                          m_timeoutRandomness;
	i32                            m_timeoutMs;
	i32                            m_runningTimeoutMs;
	i32                            m_nextUpdateMs;
	PostureUpr::PostureID      m_bestPostureID;
	PostureUpr::PostureQueryID m_queryID;

	i32  RandomizeTimeInterval() const;
	bool ProcessQueryResult(CPipeUser* pipeUser, AsyncState queryStatus, PostureUpr::PostureInfo* postureInfo);

public:
	COPDrxsis2AdjustAim(bool useLastOpAsBackup, bool allowProne, float timeout);
	explicit COPDrxsis2AdjustAim(const XmlNodeRef& node);

	virtual EGoalOpResult Execute(CPipeUser* pPipeUser);
	virtual void          Reset(CPipeUser* pPipeUser);
	virtual void          Serialize(TSerialize ser);
	virtual void          DebugDraw(CPipeUser* pPipeUser) const;
};

////////////////////////////////////////////////////////
//
//	Peek while staying in cover.
//
////////////////////////////////////////////////////////
class COPDrxsis2Peek :
	public CGoalOp
{
	CTimeValue                     m_startTime;
	CTimeValue                     m_lastGood;
	bool                           m_useLastOpAsBackup;
	float                          m_timeoutRandomness;
	i32                            m_timeoutMs;
	i32                            m_runningTimeoutMs;
	i32                            m_nextUpdateMs;
	PostureUpr::PostureID      m_bestPostureID;
	PostureUpr::PostureQueryID m_queryID;

	i32  RandomizeTimeInterval() const;
	bool ProcessQueryResult(CPipeUser* pipeUser, AsyncState queryStatus, PostureUpr::PostureInfo* postureInfo);

public:
	COPDrxsis2Peek(bool useLastOpAsBackup, float timeout);
	explicit COPDrxsis2Peek(const XmlNodeRef& node);

	virtual EGoalOpResult Execute(CPipeUser* pPipeUser);
	virtual void          Reset(CPipeUser* pPipeUser);
	virtual void          Serialize(TSerialize ser);
	virtual void          DebugDraw(CPipeUser* pPipeUser) const;
};

////////////////////////////////////////////////////////////
//
//	HIDE - makes agent find closest hiding place and then hide there
//
////////////////////////////////////////////////////////
class COPDrxsis2Hide : public CGoalOp
{
public:
	COPDrxsis2Hide(EAIRegister location, bool exact);
	explicit COPDrxsis2Hide(const XmlNodeRef& node);
	COPDrxsis2Hide(const COPDrxsis2Hide& rhs);
	virtual ~COPDrxsis2Hide();

	virtual EGoalOpResult Execute(CPipeUser* pPipeUser);

	void                  UpdateMovingToCoverAnimation(CPipeUser* pPipeUser) const;

	virtual void          ExecuteDry(CPipeUser* pPipeUser);
	virtual void          Reset(CPipeUser* pPipeUser);
	virtual void          Serialize(TSerialize ser);
	virtual void          DebugDraw(CPipeUser* pPipeUser) const;
private:
	void                  CreateHideTarget(CPipeUser* pPipeUser, const Vec3& pos, const Vec3& dir = Vec3Constants<float>::fVec3_Zero);

	CStrongRef<CAIObject> m_refHideTarget;
	EAIRegister           m_location;
	bool                  m_exact;

	COPPathFind*          m_pPathfinder;
	COPTrace*             m_pTracer;
};

////////////////////////////////////////////////////////////
//
//	COMMUNICATE - makes agent communicate (duh!)
//
////////////////////////////////////////////////////////
class COPDrxsis2Communicate : public CGoalOp
{
public:
	COPDrxsis2Communicate(CommID commID, CommChannelID channelID, float expirity, EAIRegister target,
	                      SCommunicationRequest::EOrdering ordering);
	explicit COPDrxsis2Communicate(const XmlNodeRef& node);
	virtual ~COPDrxsis2Communicate();

	EGoalOpResult Execute(CPipeUser* pPipeUser);
private:
	CommID                           m_commID;
	CommChannelID                    m_channelID;
	SCommunicationRequest::EOrdering m_ordering;

	float                            m_expirity;
	EAIRegister                      m_target;
};

////////////////////////////////////////////////////////////
//
//	STICKPATH - Follows an AI path, sticking to a target
//				projected on the path
//
////////////////////////////////////////////////////////////
class COPDrxsis2StickPath : public CGoalOp
{
public:
	COPDrxsis2StickPath(bool bFinishInRange, float fMaxStickDist, float fMinStickDist = 0.0f, bool bCanReverse = true);
	explicit COPDrxsis2StickPath(const XmlNodeRef& node);
	COPDrxsis2StickPath(const COPDrxsis2StickPath& rhs);
	virtual ~COPDrxsis2StickPath();

	virtual EGoalOpResult Execute(CPipeUser* pPipeUser);
	virtual void          ExecuteDry(CPipeUser* pPipeUser);
	virtual void          Reset(CPipeUser* pPipeUser);
	virtual void          Serialize(TSerialize ser);

private:
	// State execute helpers
	EGoalOpResult ExecuteCurrentState(CPipeUser* pPipeUser, bool bDryUpdate);
	bool          ExecuteState_Prepare(CPuppet* pPuppet, bool bDryUpdate);
	bool          ExecuteState_Navigate(CPuppet* pPuppet, bool bDryUpdate);
	bool          ExecuteState_Wait(CPuppet* pPuppet, bool bDryUpdate);

	// Project the target onto the path
	void ProjectTargetOntoPath(Vec3& outPos, float& outNearestDistance, float& outDistanceAlongPath) const;

	// Returns true if within range of target
	enum ETargetRange
	{
		eTR_TooFar,
		eTR_InRange,
		eTR_TooClose,
	};
	ETargetRange GetTargetRange(const Vec3& vMyPos) const;
	ETargetRange GetTargetRange(const Vec3& vMyPos, const Vec3& vTargetPos) const;

	// Gets the projected position to use for the vehicle based on speed settings
	Vec3 GetProjectedPos(CPuppet* pPuppet) const;

private:
	// Current state
	enum EStates
	{
		eState_FIRST,

		eState_Prepare = eState_FIRST,
		eState_Navigate,
		eState_Wait,

		eState_COUNT,
	};
	EStates m_State;

	// Current designer path shape
	SShape                m_Path;
	float                 m_fPathLength;

	CWeakRef<CAIObject>   m_refTarget;
	CStrongRef<CAIObject> m_refNavTarget;

	float                 m_fMinStickDistSq;
	float                 m_fMaxStickDistSq;
	bool                  m_bFinishInRange;
	bool                  m_bCanReverse;
};

class COPAcquirePosition : public CGoalOp
{
	enum COPAcquirePositionState
	{
		C2AP_INVALID,
		C2AP_INIT,
		C2AP_COMPUTING,
		C2AP_FAILED,
	};

	enum COPAPComputingSubstate
	{
		C2APCS_GATHERSPANS,
		C2APCS_CHECKSPANS,
		C2APCS_VISTESTS,
	};

	COPAcquirePositionState           m_State;
	COPAPComputingSubstate            m_SubState;
	EAIRegister                       m_target;
	EAIRegister                       m_output;
	Vec3                              m_curPosition;
	Vec3                              m_destination;
	std::vector<Vec3i>                m_Coords;

	//const CFlightNavRegion2::NavData* m_Graph;

public:

	COPAcquirePosition();
	explicit COPAcquirePosition(const XmlNodeRef& node);

	virtual ~COPAcquirePosition();

	EGoalOpResult Execute(CPipeUser* pPipeUser);
	void          ExecuteDry(CPipeUser* pPipeUser);
	void          Reset(CPipeUser* pPipeUser);
	bool          GetTarget(CPipeUser* pPipeUser, Vec3& target);
	bool          SetOutput(CPipeUser* pPipeUser, Vec3& target);
};

struct AStarNodeCompareFlight
{
	bool operator()(const AStarNode<Vec3i>& LHS, const AStarNode<Vec3i>& RHS) const
	{
		const Vec3i& lhs = LHS.graphNodeIndex;
		const Vec3i& rhs = RHS.graphNodeIndex;

		return lhs.x + lhs.y * 1000 + lhs.z * 1000000 < rhs.x + rhs.y * 1000 + rhs.z * 1000000;
	}
};

class NodeContainerFlight
{
	std::set<AStarNode<Vec3i>, AStarNodeCompareFlight> aStarNodes;

public:

	AStarNode<Vec3i>* GetAStarNode(Vec3i nodeIndex)
	{
		std::pair<std::set<AStarNode<Vec3i>, AStarNodeCompareFlight>::iterator, bool> result =
		  aStarNodes.insert(AStarNode<Vec3i>(nodeIndex));

		return const_cast<AStarNode<Vec3i>*>(&*result.first);
	}

	void Clear()
	{
		aStarNodes.clear();
	}
};

class ClosedListFlight
{
	std::set<Vec3i, AStarNodeCompareFlight> closedList;
public:

	void Resize(Vec3i size)
	{
		closedList.clear();
	}

	void Close(Vec3i index)
	{
		closedList.insert(index);
	}

	bool IsClosed(Vec3i index) const
	{
		return closedList.find(index) != closedList.end();
	}

	//For users debug benefit they may template specialize this function:
	void Print() const {}
};

class COPDrxsis2Fly : public CGoalOpParallel
{

	typedef CGoalOpParallel Base;

	enum COPDrxsis2FlyState
	{
		C2F_INVALID,
		C2F_PATHFINDING,
		C2F_FOLLOWINGPATH,
		C2F_FOLLOW_AND_CALC_NEXT,
		C2F_FOLLOW_AND_SWITCH,
		C2F_FAILED,
		C2F_FAILED_TEMP,
	};

	enum TargetResult
	{
		C2F_NO_TARGET,
		C2F_TARGET_FOUND,
		C2F_SET_PATH,
	};

	struct Threat
	{
		EntityId threatId;
		Vec3     threatPos;
		Vec3     threatDir;

		Threat() { Reset(); }

		void Reset() { threatId = 0; threatPos = ZERO; threatDir = ZERO; }
	};

	COPDrxsis2FlyState m_State;
	EAIRegister        m_target;
	Vec3               m_CurSegmentDir;
	Vec3               m_destination;
	Vec3               m_nextDestination;
	u32             m_Length;
	u32             m_PathSizeFinish;
	float              m_Timeout;
	float              m_lookAheadDist;
	float              m_desiredSpeed;
	float              m_currentSpeed;

	float              m_minDistance;
	float              m_maxDistance;
	i32                m_TargetEntity;

	Threat             m_Threat;

	bool               m_Circular;
public:

	//typedef GenericAStarSolver<CFlightNavRegion2::NavData, CFlightNavRegion2::NavData, CFlightNavRegion2::NavData, DefaultOpenList<Vec3i>, ClosedListFlight, NodeContainerFlight> ASTARSOLVER;
	//typedef stl::PoolAllocator<sizeof(ASTARSOLVER)>                                                                                                                               SolverAllocator;

private:

	std::vector<Vec3>                 m_PathOut;
	std::vector<Vec3>                 m_Reversed;

	/*ASTARSOLVER*                      m_Solver;

	const CFlightNavRegion2::NavData* m_Graph;

	static SolverAllocator            m_Solvers;

	ASTARSOLVER* AllocateSolver()
	{
		return new(m_Solvers.Allocate())ASTARSOLVER;
	}

	void DestroySolver(ASTARSOLVER* solver)
	{
		solver->~ASTARSOLVER();
		m_Solvers.Deallocate(solver);
	}*/

	TargetResult  GetTarget(CPipeUser* pPipeUser, Vec3& target) const;
	EGoalOpResult CalculateTarget(CPipeUser* pPipeUser);
	bool          HandleThreat(CPipeUser* pPipeUser, const Vec3& lookAheadPos);

public:

	COPDrxsis2Fly();
	COPDrxsis2Fly(const COPDrxsis2Fly& rhs);
	explicit COPDrxsis2Fly(const XmlNodeRef& node);

	virtual ~COPDrxsis2Fly();

	EGoalOpResult Execute(CPipeUser* pPipeUser);
	void          ExecuteDry(CPipeUser* pPipeUser);
	void          Reset(CPipeUser* pPipeUser);

	void          ParseParam(tukk param, const GoalParams& value);
	void          SendEvent(CPipeUser* pPipeUser, tukk eventName);

	float         CheckTargetEntity(const CPipeUser* pPipeUser, const Vec3& lookAheadPos);

	void          Serialize(TSerialize ser);

};

class COPDrxsis2ChaseTarget : public CGoalOpParallel
{
	typedef CGoalOpParallel Base;

	enum COPDrxsis2ChaseTargetState
	{
		C2F_INVALID,
		C2F_FINDINGLOCATION,
		C2F_WAITING_LOCATION_RESULT,
		C2F_PREPARING,
		C2F_CHASING,
		C2F_FAILED,
		C2F_FAILED_TEMP,
	};

	enum TargetResult
	{
		C2F_NO_TARGET,
		C2F_TARGET_FOUND,
		C2F_SET_PATH,
	};

	struct CandidateLocation
	{
		Vec3   point;
		size_t index;
		float  fraction;

		float  score;

		CandidateLocation()
		{
			// nothing
		}

		explicit CandidateLocation(type_zero)
			: point(Vec3Constants<float>::fVec3_Zero)
			, index(0)
			, fraction(0.0f)
			, score(0.0f)
		{
		}

		bool operator<(const CandidateLocation& other) const
		{
			return score > other.score;
		}
	};

	typedef std::vector<CandidateLocation> Candidates;

	COPDrxsis2ChaseTargetState m_State;
	EAIRegister                m_target;
	Vec3                       m_destination;
	Vec3                       m_nextDestination;
	float                      m_lookAheadDist;
	float                      m_desiredSpeed;

	float                      m_distanceMin;
	float                      m_distanceMax;

	QueuedRayID                m_visCheckRayID[2];
	size_t                     m_visCheckResultCount;
	CandidateLocation          m_bestLocation;
	CandidateLocation          m_testLocation;

	ListPositions              m_PathOut;
	ListPositions              m_throughOriginBuf;

	GoalParams                 m_firePauseParams;

	TargetResult  GetTarget(CPipeUser* pPipeUser, Vec3& target, IAIObject** targetObject = 0) const;
	EGoalOpResult Chase(CPipeUser* pPipeUser);
	void          TestLocation(const CandidateLocation& location, CPipeUser* pipeUser, const Vec3& target);

	size_t        GatherCandidateLocations(float currentLocationAlong, const ListPositions& path, float pathLength,
	                                       bool closed, float spacing, const Vec3& target, Candidates& candidates);
	Candidates m_candidates;

	void  VisCheckRayComplete(const QueuedRayID& rayID, const RayCastResult& result);

	float CalculateShortestPathTo(const ListPositions& path, bool closed, size_t startIndex, float startSegmentFraction,
	                              size_t endIndex, float endSegmentFraction, ListPositions& pathOut);

public:
	COPDrxsis2ChaseTarget();
	COPDrxsis2ChaseTarget(const COPDrxsis2ChaseTarget& rhs);
	explicit COPDrxsis2ChaseTarget(const XmlNodeRef& node);

	virtual ~COPDrxsis2ChaseTarget();

	EGoalOpResult Execute(CPipeUser* pPipeUser);
	void          ExecuteDry(CPipeUser* pPipeUser);
	void          Reset(CPipeUser* pPipeUser);

	void          ParseParam(tukk param, const GoalParams& value);
};

class COPDrxsis2FlightFireWeapons : public CGoalOpParallel
{
	enum
	{
		AI_REG_INTERNAL_TARGET = AI_REG_LAST + 1,
	};

	enum COPDrxsis2FlightFireWeaponsState
	{
		eFP_START,
		eFP_STOP,
		eFP_DONE,
		eFP_WAIT,
		eFP_WAIT_INIT,
		eFP_PREPARE,
		eFP_PAUSED,
		eFP_PAUSED_OVERRIDE,
		eFP_RUN_INDEFINITELY,
	};

	COPDrxsis2FlightFireWeaponsState m_State;
	COPDrxsis2FlightFireWeaponsState m_NextState;
	Vec3                             m_LastTarget;
	float                            m_InitCoolDown;
	float                            m_WaitTime;
	float                            m_minTime;
	float                            m_maxTime;
	float                            m_PausedTime;
	float                            m_PauseOverrideTime;
	float                            m_Rotation;
	u32                           m_SecondaryWeapons;
	u32                           m_targetId;
	EAIRegister                      m_target;
	bool                             m_FirePrimary;
	bool                             m_OnlyLookAt;

public:

	COPDrxsis2FlightFireWeapons();
	explicit COPDrxsis2FlightFireWeapons(const XmlNodeRef& node);
	COPDrxsis2FlightFireWeapons(EAIRegister reg, float minTime, float maxTime, bool primary, u32 secondary);

	virtual ~COPDrxsis2FlightFireWeapons();

	EGoalOpResult Execute(CPipeUser* pPipeUser);
	void          ExecuteDry(CPipeUser* pPipeUser);
	void          ExecuteShoot(CPipeUser* pPipeUser, const Vec3& targetPos);
	void          Reset(CPipeUser* pPipeUser);
	bool          GetTarget(CPipeUser* pPipeUser, Vec3& target);

	void          ParseParam(tukk param, const GoalParams& value);

	bool          CalculateHitPosPlayer(CPipeUser* pPipeUser, CAIPlayer* targetPlayer, Vec3& target);
	bool          CalculateHitPos(CPipeUser* pPipeUser, CAIObject* targetObject, Vec3& target);

	void          Serialize(TSerialize ser);

};

class COPDrxsis2Hover : public CGoalOp
{
	enum COPHoverState
	{
		CHS_INVALID,
		CHS_HOVERING,
		CHS_HOVERING_APPROACH,
	};

	Vec3          m_InitialPos;
	Vec3          m_InitialForward;
	Vec3          m_InitialTurn;
	Vec3          m_CurrentTarget;
	EAIRegister   m_target;
	bool          m_Continuous;

	COPHoverState m_State;
	COPHoverState m_NextState;
public:

	COPDrxsis2Hover();
	COPDrxsis2Hover(EAIRegister reg, bool continuous);
	explicit COPDrxsis2Hover(const XmlNodeRef& node);

	virtual ~COPDrxsis2Hover();

	void          Reset(CPipeUser* pPipeUser);
	EGoalOpResult Execute(CPipeUser* pPipeUser);
	void          ExecuteDry(CPipeUser* pPipeUser);
	bool          GetTarget(CPipeUser* pPipeUser, Vec3& target);

};
#endif //__GoalOp_Drxsis2_H__
