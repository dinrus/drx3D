// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __PostureUpr_h__
#define __PostureUpr_h__

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/AI/Cover.h>

class PostureUpr
{
public:
	typedef i32  PostureID;
	typedef u32 PostureQueryID;

	enum PostureType
	{
		InvalidPosture = 0,
		PeekPosture    = 1,
		AimPosture     = 2,
		HidePosture    = 3,
	};

	enum PostureQueryChecks
	{
		CheckVisibility   = 1 << 0,
		CheckAimability   = 1 << 1,
		CheckLeanability  = 1 << 2,
		CheckFriendlyFire = 1 << 3,
		DefaultChecks     = CheckVisibility | CheckAimability | CheckLeanability | CheckFriendlyFire,
	};

	struct PostureQuery
	{
		PostureQuery()
			: position(ZERO)
			, target(ZERO)
			, actor(0)
			, distancePercent(0.5f)
			, coverID()
			, checks(DefaultChecks)
			, hintPostureID(-1)
			, type(InvalidPosture)
			, allowLean(true)
			, allowProne(false)
			, stickyStance(true)
		{
		}

		Vec3        position;
		Vec3        target;

		CAIActor*   actor;

		float       distancePercent; // distance from actor to target to use for checks
		CoverID     coverID;         // currenct actor cover

		u32      checks;        // which checks to perform
		PostureID   hintPostureID; // current posture - check this first
		PostureType type;
		bool        allowLean;
		bool        allowProne;
		bool        stickyStance;
	};

	struct PostureInfo
	{
		PostureInfo()
			: type(InvalidPosture)
			, lean(0.0f)
			, peekOver(0.0f)
			, stance(STANCE_NULL)
			, priority(0.0f)
			, minDistanceToTarget(0.0f)
			, maxDistanceToTarget(FLT_MAX)
			, parentID(-1)
			, enabled(true)
		{}

		PostureInfo(PostureType _type, tukk _name, float _lean, float _peekOver, EStance _stance, float _priority, tukk _agInput = "", PostureID _parentID = -1)
			: type(_type)
			, lean(_lean)
			, peekOver(_peekOver)
			, stance(_stance)
			, priority(_priority)
			, minDistanceToTarget(0.0f)
			, maxDistanceToTarget(FLT_MAX)
			, parentID(_parentID)
			, enabled(true)
			, agInput(_agInput)
			, name(_name)
		{
		}

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(agInput);
			pSizer->AddObject(name);
		}

		PostureType type;
		float       lean;
		float       peekOver;
		float       priority;
		float       minDistanceToTarget;
		float       maxDistanceToTarget;
		EStance     stance;
		PostureID   parentID;
		bool        enabled;
		string      agInput;
		string      name;
	};

	PostureUpr();
	~PostureUpr();

	void           ResetPostures();
	void           AddDefaultPostures(PostureType type);

	PostureID      AddPosture(const PostureInfo& posture);
	void           SetPosture(PostureID postureID, const PostureInfo& posture);
	bool           GetPosture(PostureID postureID, PostureInfo* posture = 0) const;
	PostureID      GetPostureID(tukk postureName) const;
	bool           GetPostureByName(tukk postureName, PostureInfo* posture = 0) const;
	void           SetPosturePriority(PostureID postureID, float priority);
	float          GetPosturePriority(PostureID postureID) const;

	PostureQueryID QueryPosture(const PostureQuery& postureQuery);
	void           CancelPostureQuery(PostureQueryID queryID);
	AsyncState     GetPostureQueryResult(PostureQueryID queryID, PostureID* postureID, PostureInfo** postureInfo);

private:
	typedef std::vector<PostureInfo> PostureInfos;
	PostureInfos m_postureInfos;

	struct RunningPosture
	{
		RunningPosture(i16 _postureID = -1)
			: postureID(_postureID)
			, targetVis(false)
			, targetAim(false)
			, eye(ZERO)
			, weapon(ZERO)
			, processed(false)
		{
		}

		Vec3      eye;
		Vec3      weapon;

		PostureID postureID;
		bool      targetVis : 1;
		bool      targetAim : 1;
		bool      processed : 1;
	};

	struct PostureSorter
	{
		PostureSorter(const PostureUpr& _manager, PostureID _hintPostureID = -1)
			: manager(_manager)
			, hintPostureID(_hintPostureID)
		{
		}

		bool operator()(const RunningPosture& lhs, const RunningPosture& rhs) const
		{
			if (hintPostureID == lhs.postureID)
				return true;

			if (hintPostureID == rhs.postureID)
				return false;

			return CAISystem::CompareFloatsFPUBugWorkaround(manager.GetPosturePriority(lhs.postureID),
			                                                manager.GetPosturePriority(rhs.postureID));
		}

		PostureID             hintPostureID;
		const PostureUpr& manager;
	};

	struct StickyStancePostureSorter
	{
		StickyStancePostureSorter(const PostureUpr& _manager, const PostureInfos& _infos, EStance _stickyStance,
		                          PostureID _hintPostureID = -1)
			: manager(_manager)
			, infos(_infos)
			, stickyStance(_stickyStance)
			, hintPostureID(_hintPostureID)
		{
		}

		bool operator()(const RunningPosture& lhs, const RunningPosture& rhs) const
		{
			if (hintPostureID == lhs.postureID)
				return true;

			if (hintPostureID == rhs.postureID)
				return false;

			EStance lhsStance = infos[lhs.postureID].stance;
			EStance rhsStance = infos[rhs.postureID].stance;

			if (lhsStance != rhsStance)
			{
				if (lhsStance == stickyStance)
					return true;
				else if (rhsStance == stickyStance)
					return false;
			}

			return CAISystem::CompareFloatsFPUBugWorkaround(manager.GetPosturePriority(lhs.postureID),
			                                                manager.GetPosturePriority(rhs.postureID));
		}

		const PostureInfos&   infos;
		EStance               stickyStance;
		PostureID             hintPostureID;
		const PostureUpr& manager;
	};

	struct QueuedPostureCheck
	{
		QueuedRayID leanabilityRayID;
		QueuedRayID visibilityRayID;
		QueuedRayID aimabilityRayID;

		PostureID   postureID;

		u8       awaitingResultCount;
		u8       positiveResultCount;
	};

	typedef std::vector<QueuedPostureCheck> QueuedPostureChecks;

	struct QueuedQuery
	{
		QueuedQuery()
			: queryID(0)
			, status(AsyncFailed)
			, result(-1)
		{
		}

		PostureQueryID      queryID;
		AsyncState          status;
		PostureID           result;
		QueuedPostureChecks postureChecks;
	};

	typedef std::vector<QueuedQuery> QueuedPostureQueries;
	QueuedPostureQueries m_queue;

	PostureQueryID       m_queryGenID;
	u32               m_queueTail;
	u32               m_queueSize;

	enum
	{
		TotalCheckCount = 3,
	};
	void CancelRays(QueuedQuery& query);
	void RayComplete(const QueuedRayID& rayID, const RayCastResult& result);
};

#endif
