
#if 0

#include <drx3D/b3ContactCache.h>
#include <drx3D/Common/b3Transform.h>

#include <drx3D/Physics/Collision/NarrowPhase/shared/b3Contact4Data.h>

b3Scalar					gContactBreakingThreshold = b3Scalar(0.02);

///gContactCalcArea3Points will approximate the convex hull area using 3 points
///when setting it to false, it will use 4 points to compute the area: it is more accurate but slower
bool						gContactCalcArea3Points = true;




static inline b3Scalar calcArea4Points(const b3Vec3 &p0,const b3Vec3 &p1,const b3Vec3 &p2,const b3Vec3 &p3)
{
	// It calculates possible 3 area constructed from random 4 points and returns the biggest one.

	b3Vec3 a[3],b[3];
	a[0] = p0 - p1;
	a[1] = p0 - p2;
	a[2] = p0 - p3;
	b[0] = p2 - p3;
	b[1] = p1 - p3;
	b[2] = p1 - p2;

	//todo: Following 3 cross production can be easily optimized by SIMD.
	b3Vec3 tmp0 = a[0].cross(b[0]);
	b3Vec3 tmp1 = a[1].cross(b[1]);
	b3Vec3 tmp2 = a[2].cross(b[2]);

	return d3Max(d3Max(tmp0.length2(),tmp1.length2()),tmp2.length2());
}
#if 0

//using localPointA for all points
i32 b3ContactCache::sortCachedPoints(const b3Vec3& pt)
{
		//calculate 4 possible cases areas, and take biggest area
		//also need to keep 'deepest'

		i32 maxPenetrationIndex = -1;
#define KEEP_DEEPEST_POINT 1
#ifdef KEEP_DEEPEST_POINT
		b3Scalar maxPenetration = pt.getDistance();
		for (i32 i=0;i<4;i++)
		{
			if (m_pointCache[i].getDistance() < maxPenetration)
			{
				maxPenetrationIndex = i;
				maxPenetration = m_pointCache[i].getDistance();
			}
		}
#endif  //KEEP_DEEPEST_POINT

		b3Scalar res0(b3Scalar(0.)),res1(b3Scalar(0.)),res2(b3Scalar(0.)),res3(b3Scalar(0.));

	if (gContactCalcArea3Points)
	{
		if (maxPenetrationIndex != 0)
		{
			b3Vec3 a0 = pt.m_localPointA-m_pointCache[1].m_localPointA;
			b3Vec3 b0 = m_pointCache[3].m_localPointA-m_pointCache[2].m_localPointA;
			b3Vec3 cross = a0.cross(b0);
			res0 = cross.length2();
		}
		if (maxPenetrationIndex != 1)
		{
			b3Vec3 a1 = pt.m_localPointA-m_pointCache[0].m_localPointA;
			b3Vec3 b1 = m_pointCache[3].m_localPointA-m_pointCache[2].m_localPointA;
			b3Vec3 cross = a1.cross(b1);
			res1 = cross.length2();
		}

		if (maxPenetrationIndex != 2)
		{
			b3Vec3 a2 = pt.m_localPointA-m_pointCache[0].m_localPointA;
			b3Vec3 b2 = m_pointCache[3].m_localPointA-m_pointCache[1].m_localPointA;
			b3Vec3 cross = a2.cross(b2);
			res2 = cross.length2();
		}

		if (maxPenetrationIndex != 3)
		{
			b3Vec3 a3 = pt.m_localPointA-m_pointCache[0].m_localPointA;
			b3Vec3 b3 = m_pointCache[2].m_localPointA-m_pointCache[1].m_localPointA;
			b3Vec3 cross = a3.cross(b3);
			res3 = cross.length2();
		}
	}
	else
	{
		if(maxPenetrationIndex != 0) {
			res0 = calcArea4Points(pt.m_localPointA,m_pointCache[1].m_localPointA,m_pointCache[2].m_localPointA,m_pointCache[3].m_localPointA);
		}

		if(maxPenetrationIndex != 1) {
			res1 = calcArea4Points(pt.m_localPointA,m_pointCache[0].m_localPointA,m_pointCache[2].m_localPointA,m_pointCache[3].m_localPointA);
		}

		if(maxPenetrationIndex != 2) {
			res2 = calcArea4Points(pt.m_localPointA,m_pointCache[0].m_localPointA,m_pointCache[1].m_localPointA,m_pointCache[3].m_localPointA);
		}

		if(maxPenetrationIndex != 3) {
			res3 = calcArea4Points(pt.m_localPointA,m_pointCache[0].m_localPointA,m_pointCache[1].m_localPointA,m_pointCache[2].m_localPointA);
		}
	}
	b3Vec4 maxvec(res0,res1,res2,res3);
	i32 biggestarea = maxvec.closestAxis4();
	return biggestarea;

}


i32 b3ContactCache::getCacheEntry(const b3Vec3& newPoint) const
{
	b3Scalar shortestDist =  getContactBreakingThreshold() * getContactBreakingThreshold();
	i32 size = getNumContacts();
	i32 nearestPoint = -1;
	for( i32 i = 0; i < size; i++ )
	{
		const b3Vec3 &mp = m_pointCache[i];

		b3Vec3 diffA =  mp.m_localPointA- newPoint.m_localPointA;
		const b3Scalar distToManiPoint = diffA.dot(diffA);
		if( distToManiPoint < shortestDist )
		{
			shortestDist = distToManiPoint;
			nearestPoint = i;
		}
	}
	return nearestPoint;
}

i32 b3ContactCache::addManifoldPoint(const b3Vec3& newPoint)
{
	drx3DAssert(validContactDistance(newPoint));

	i32 insertIndex = getNumContacts();
	if (insertIndex == MANIFOLD_CACHE_SIZE)
	{
#if MANIFOLD_CACHE_SIZE >= 4
		//sort cache so best points come first, based on area
		insertIndex = sortCachedPoints(newPoint);
#else
		insertIndex = 0;
#endif
		clearUserCache(m_pointCache[insertIndex]);

	} else
	{
		m_cachedPoints++;


	}
	if (insertIndex<0)
		insertIndex=0;

	//drx3DAssert(m_pointCache[insertIndex].m_userPersistentData==0);
	m_pointCache[insertIndex] = newPoint;
	return insertIndex;
}

#endif

bool b3ContactCache::validContactDistance(const b3Vec3& pt)
{
	return pt.w <= gContactBreakingThreshold;
}

void b3ContactCache::removeContactPoint(struct b3Contact4Data& newContactCache,i32 i)
{
	i32 numContacts = b3Contact4Data_getNumPoints(&newContactCache);
	if (i!=(numContacts-1))
	{
		b3Swap(newContactCache.m_localPosA[i],newContactCache.m_localPosA[numContacts-1]);
		b3Swap(newContactCache.m_localPosB[i],newContactCache.m_localPosB[numContacts-1]);
		b3Swap(newContactCache.m_worldPosB[i],newContactCache.m_worldPosB[numContacts-1]);
	}
	b3Contact4Data_setNumPoints(&newContactCache,numContacts-1);

}


void b3ContactCache::refreshContactPoints(const b3Transform& trA,const b3Transform& trB, struct b3Contact4Data& contacts)
{

	i32 numContacts = b3Contact4Data_getNumPoints(&contacts);


	i32 i;
	/// first refresh worldspace positions and distance
	for (i=numContacts-1;i>=0;i--)
	{
		b3Vec3 worldPosA = trA( contacts.m_localPosA[i]);
		b3Vec3 worldPosB = trB( contacts.m_localPosB[i]);
		contacts.m_worldPosB[i] = worldPosB;
		float distance = (worldPosA -  worldPosB).dot(contacts.m_worldNormalOnB);
		contacts.m_worldPosB[i].w = distance;
	}

	/// then
	b3Scalar distance2d;
	b3Vec3 projectedDifference,projectedPoint;
	for (i=numContacts-1;i>=0;i--)
	{
		b3Vec3 worldPosA = trA( contacts.m_localPosA[i]);
		b3Vec3 worldPosB = trB( contacts.m_localPosB[i]);
		b3Vec3&pt = contacts.m_worldPosB[i];
		//contact becomes invalid when signed distance exceeds margin (projected on contactnormal direction)
		if (!validContactDistance(pt))
		{
			removeContactPoint(contacts,i);
		} else
		{
			//contact also becomes invalid when relative movement orthogonal to normal exceeds margin
			projectedPoint = worldPosA - contacts.m_worldNormalOnB * contacts.m_worldPosB[i].w;
			projectedDifference = contacts.m_worldPosB[i] - projectedPoint;
			distance2d = projectedDifference.dot(projectedDifference);
			if (distance2d  > gContactBreakingThreshold*gContactBreakingThreshold )
			{
				removeContactPoint(contacts,i);
			} else
			{
				////contact point processed callback
				//if (gContactProcessedCallback)
				//	(*gContactProcessedCallback)(manifoldPoint,(uk )m_body0,(uk )m_body1);
			}
		}
	}


}

#endif
