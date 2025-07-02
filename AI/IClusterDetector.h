// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IClusterDetector
//  Version:     v1.00
//  Created:     01/01/2008 by Francesco Roccucci.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __I_Cluster_Detector_h__
#define __I_Cluster_Detector_h__

typedef u32 ClusterPointProperties;
typedef u32 ClusterId;

struct ClusterPoint
{
	ClusterPoint()
		: pos(ZERO)
		, clusterId(~0)
		, pointId(0)
	{
	}

	ClusterPoint(u32k _id, const Vec3& _pos)
		: pos(_pos)
		, clusterId(~0)
		, pointId(_id)
	{
	}

	ClusterPoint(const ClusterPoint& _clusterPoint)
		: pos(_clusterPoint.pos)
		, clusterId(_clusterPoint.clusterId)
		, pointId(_clusterPoint.pointId)
	{
	}

	Vec3      pos;
	ClusterId clusterId;

	//! This is needed for the requesters to be able to match the point with the entity they have.
	u32 pointId;

};

struct IClusterRequest
{
	typedef Functor1<IClusterRequest*> Callback;
	// <interfuscator:shuffle>
	virtual ~IClusterRequest() {};
	virtual void                SetNewPointInRequest(u32k pointId, const Vec3& location) = 0;
	virtual void                SetCallback(Callback callback) = 0;
	virtual size_t              GetNumberOfPoint() const = 0;
	virtual void                SetMaximumSqDistanceAllowedPerCluster(float maxDistance) = 0;
	virtual const ClusterPoint* GetPointAt(const size_t pointIndex) const = 0;
	virtual size_t              GetTotalClustersNumber() const = 0;
	// </interfuscator:shuffle>
};

struct IClusterDetector
{
	typedef u32                                  ClusterRequestID;
	typedef std::pair<ClusterRequestID, IClusterRequest*> IClusterRequestPair;
	// <interfuscator:shuffle>
	virtual ~IClusterDetector() {}
	virtual IClusterRequestPair CreateNewRequest() = 0;
	virtual void                QueueRequest(const ClusterRequestID requestId) = 0;
	// </interfuscator:shuffle>
};

#endif // __I_Cluster_Detector_h__
