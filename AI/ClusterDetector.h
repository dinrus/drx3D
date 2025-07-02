// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ClusterDetector.h
//  Version:     v1.00
//  Created:     01/01/2008 by Francesco Roccucci.
//  Описание: This is the actual class that implements the detection of
//               the different clusters in the world space.
//               This suystem only knows about the presence of points into
//               the space and he group them into clusters
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ClusterDetector_h__
#define __ClusterDetector_h__

#include <drx3D/AI/IClusterDetector.h>

struct Cluster
{
	typedef float                                 DistanceSqFromCenter;
	typedef std::pair<DistanceSqFromCenter, Vec3> DistancePointPair;

	Cluster()
		: centerPos(ZERO)
		, numberOfElements(0)
		, pointsWeight(1.0f)
		, pointAtMaxDistanceSqFromCenter(FLT_MIN, Vec3(ZERO))
	{
	}

	Cluster(const Vec3& _pos)
		: centerPos(_pos)
		, numberOfElements(0)
		, pointsWeight(1.0f)
		, pointAtMaxDistanceSqFromCenter(FLT_MIN, Vec3(ZERO))
	{
	}

	Vec3              centerPos;
	u8             numberOfElements;
	float             pointsWeight;
	DistancePointPair pointAtMaxDistanceSqFromCenter;
};

struct ClusterRequest : public IClusterRequest
{

	enum EClusterRequestState
	{
		eRequestState_Created,
		eRequestState_ReadyToBeProcessed,
	};

	ClusterRequest();

	virtual void                SetNewPointInRequest(u32k pointId, const Vec3& location);
	virtual size_t              GetNumberOfPoint() const;
	virtual void                SetMaximumSqDistanceAllowedPerCluster(float maxDistanceSq);
	virtual void                SetCallback(Callback callback);
	virtual const ClusterPoint* GetPointAt(const size_t pointIndex) const;
	virtual size_t              GetTotalClustersNumber() const;

	typedef std::vector<ClusterPoint> PointsList;
	PointsList           m_points;
	Callback             m_callback;
	EClusterRequestState m_state;
	float                m_maxDistanceSq;
	size_t               m_totalClustersNumber;
};

class ClusterDetector : public IClusterDetector
{
	typedef std::vector<Cluster> ClustersArray;

	struct CurrentRequestState
	{
		CurrentRequestState()
			: pCurrentRequest(NULL)
			, currentRequestId(~0)
			, aabbCenter(ZERO)
			, spawningPositionForNextCluster(ZERO)
		{
		}

		void Reset();

		ClusterRequest*  pCurrentRequest;
		ClusterRequestID currentRequestId;
		ClustersArray    clusters;
		Vec3             aabbCenter;
		Vec3             spawningPositionForNextCluster;
	};

public:

	enum EStatus
	{
		eStatus_Waiting = 0,
		eStatus_ClusteringInProgress,
		eStatus_ClusteringCompleted,
	};

	ClusterDetector();

	virtual IClusterRequestPair CreateNewRequest();
	virtual void                QueueRequest(const ClusterRequestID requestId);

	void                        Update(float frameDeltaTime);
	void                        Reset();

	typedef std::pair<ClusterRequestID, ClusterRequest> ClusterRequestPair;

private:

	bool                InitializeNextRequest();

	ClusterRequestID    GenerateUniqueClusterRequestId();
	ClusterRequestPair* ChooseRequestToServe();
	size_t              CalculateRuleOfThumbClusterSize(const size_t numberOfPoints);
	void                InitializeClusters();
	Vec3                CalculateInitialClusterPosition(const size_t clusterIndex, const float spreadAngleUnit) const;

	// Steps
	bool ExecuteClusteringStep();
	void UpdateClustersCenter();
	void ResetClustersElements();
	bool AddNewCluster();
	bool IsNewClusterNeeded();

private:
	typedef std::list<ClusterRequestPair> RequestsList;
	CurrentRequestState m_internalState;
	ClusterRequestID    m_nextUniqueRequestID;
	RequestsList        m_requests;
	EStatus             m_status;
};

#endif // __ClusterDetector_h__
