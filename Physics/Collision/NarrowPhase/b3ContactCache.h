#ifndef D3_CONTACT_CACHE_H
#define D3_CONTACT_CACHE_H

#include <drx3D/Common/b3Vec3.h>
#include <drx3D/Common/b3Transform.h>
#include <drx3D/Common/b3AlignedAllocator.h>

///maximum contact breaking and merging threshold
extern b3Scalar gContactBreakingThreshold;

#define MANIFOLD_CACHE_SIZE 4

///b3ContactCache is a contact point cache, it stays persistent as long as objects are overlapping in the broadphase.
///Those contact points are created by the collision narrow phase.
///The cache can be empty, or hold 1,2,3 or 4 points. Some collision algorithms (GJK) might only add one point at a time.
///updates/refreshes old contact points, and throw them away if necessary (distance becomes too large)
///reduces the cache to 4 points, when more then 4 points are added, using following rules:
///the contact point with deepest penetration is always kept, and it tries to maximuze the area covered by the points
///note that some pairs of objects might have more then one contact manifold.
D3_ATTRIBUTE_ALIGNED16(class)
b3ContactCache
{
	/// sort cached points so most isolated points come first
	i32 sortCachedPoints(const b3Vec3& pt);

public:
	D3_DECLARE_ALIGNED_ALLOCATOR();

	i32 addManifoldPoint(const b3Vec3& newPoint);

	/*void replaceContactPoint(const b3Vec3& newPoint,i32 insertIndex)
	{
		drx3DAssert(validContactDistance(newPoint));
		m_pointCache[insertIndex] = newPoint;
	}
	*/

	static bool validContactDistance(const b3Vec3& pt);

	/// calculated new worldspace coordinates and depth, and reject points that exceed the collision margin
	static void refreshContactPoints(const b3Transform& trA, const b3Transform& trB, struct b3Contact4Data& newContactCache);

	static void removeContactPoint(struct b3Contact4Data & newContactCache, i32 i);
};

#endif  //D3_CONTACT_CACHE_H
