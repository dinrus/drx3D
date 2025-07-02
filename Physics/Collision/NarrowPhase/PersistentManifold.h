#ifndef DRX3D_PERSISTENT_MANIFOLD_H
#define DRX3D_PERSISTENT_MANIFOLD_H

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Physics/Collision/NarrowPhase/ManifoldPoint.h>
class CollisionObject2;
#include <drx3D/Maths/Linear/AlignedAllocator.h>

struct CollisionResult;
struct CollisionObject2DoubleData;
struct CollisionObject2FloatData;

///maximum contact breaking and merging threshold
extern Scalar gContactBreakingThreshold;

#ifndef SWIG
class PersistentManifold;

typedef bool (*ContactDestroyedCallback)(uk userPersistentData);
typedef bool (*ContactProcessedCallback)(ManifoldPoint& cp, uk body0, uk body1);
typedef void (*ContactStartedCallback)(PersistentManifold* const& manifold);
typedef void (*ContactEndedCallback)(PersistentManifold* const& manifold);
extern ContactDestroyedCallback gContactDestroyedCallback;
extern ContactProcessedCallback gContactProcessedCallback;
extern ContactStartedCallback gContactStartedCallback;
extern ContactEndedCallback gContactEndedCallback;
#endif  //SWIG

//the enum starts at 1024 to avoid type conflicts with TypedConstraint
enum ContactManifoldTypes
{
	MIN_CONTACT_MANIFOLD_TYPE = 1024,
	DRX3D_PERSISTENT_MANIFOLD_TYPE
};

#define MANIFOLD_CACHE_SIZE 4

//PersistentManifold is a contact point cache, it stays persistent as long as objects are overlapping in the broadphase.
///Those contact points are created by the collision narrow phase.
///The cache can be empty, or hold 1,2,3 or 4 points. Some collision algorithms (GJK) might only add one point at a time.
///updates/refreshes old contact points, and throw them away if necessary (distance becomes too large)
///reduces the cache to 4 points, when more then 4 points are added, using following rules:
///the contact point with deepest penetration is always kept, and it tries to maximuze the area covered by the points
///note that some pairs of objects might have more then one contact manifold.

//ATTRIBUTE_ALIGNED128( class) PersistentManifold : public TypedObject
ATTRIBUTE_ALIGNED16(class)
PersistentManifold : public TypedObject
{
	ManifoldPoint m_pointCache[MANIFOLD_CACHE_SIZE];

	/// this two body pointers can point to the physics rigidbody class.
	const CollisionObject2* m_body0;
	const CollisionObject2* m_body1;

	i32 m_cachedPoints;

	Scalar m_contactBreakingThreshold;
	Scalar m_contactProcessingThreshold;

	/// sort cached points so most isolated points come first
	i32 sortCachedPoints(const ManifoldPoint& pt);

	i32 findContactPoint(const ManifoldPoint* unUsed, i32 numUnused, const ManifoldPoint& pt);

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	i32 m_companionIdA;
	i32 m_companionIdB;

	i32 m_index1a;

	PersistentManifold();

	PersistentManifold(const CollisionObject2* body0, const CollisionObject2* body1, i32, Scalar contactBreakingThreshold, Scalar contactProcessingThreshold)
		: TypedObject(DRX3D_PERSISTENT_MANIFOLD_TYPE),
		  m_body0(body0),
		  m_body1(body1),
		  m_cachedPoints(0),
		  m_contactBreakingThreshold(contactBreakingThreshold),
		  m_contactProcessingThreshold(contactProcessingThreshold),
		  m_companionIdA(0),
		  m_companionIdB(0),
		  m_index1a(0)
	{
	}

	SIMD_FORCE_INLINE const CollisionObject2* getBody0() const { return m_body0; }
	SIMD_FORCE_INLINE const CollisionObject2* getBody1() const { return m_body1; }

	void setBodies(const CollisionObject2* body0, const CollisionObject2* body1)
	{
		m_body0 = body0;
		m_body1 = body1;
	}

	void clearUserCache(ManifoldPoint & pt);

#ifdef DEBUG_PERSISTENCY
	void DebugPersistency();
#endif  //

	SIMD_FORCE_INLINE i32 getNumContacts() const
	{
		return m_cachedPoints;
	}
	/// the setNumContacts API is usually not used, except when you gather/fill all contacts manually
	void setNumContacts(i32 cachedPoints)
	{
		m_cachedPoints = cachedPoints;
	}

	SIMD_FORCE_INLINE const ManifoldPoint& getContactPoint(i32 index) const
	{
		Assert(index < m_cachedPoints);
		return m_pointCache[index];
	}

	SIMD_FORCE_INLINE ManifoldPoint& getContactPoint(i32 index)
	{
		Assert(index < m_cachedPoints);
		return m_pointCache[index];
	}

	///@todo: get this margin from the current physics / collision environment
	Scalar getContactBreakingThreshold() const;

	Scalar getContactProcessingThreshold() const
	{
		return m_contactProcessingThreshold;
	}

	void setContactBreakingThreshold(Scalar contactBreakingThreshold)
	{
		m_contactBreakingThreshold = contactBreakingThreshold;
	}

	void setContactProcessingThreshold(Scalar contactProcessingThreshold)
	{
		m_contactProcessingThreshold = contactProcessingThreshold;
	}

	i32 getCacheEntry(const ManifoldPoint& newPoint) const;

	i32 addManifoldPoint(const ManifoldPoint& newPoint, bool isPredictive = false);

	void removeContactPoint(i32 index)
	{
		clearUserCache(m_pointCache[index]);

		i32 lastUsedIndex = getNumContacts() - 1;
		//		m_pointCache[index] = m_pointCache[lastUsedIndex];
		if (index != lastUsedIndex)
		{
			m_pointCache[index] = m_pointCache[lastUsedIndex];
			//get rid of duplicated userPersistentData pointer
			m_pointCache[lastUsedIndex].m_userPersistentData = 0;
			m_pointCache[lastUsedIndex].m_appliedImpulse = 0.f;
			m_pointCache[lastUsedIndex].m_prevRHS = 0.f;
			m_pointCache[lastUsedIndex].m_contactPointFlags = 0;
			m_pointCache[lastUsedIndex].m_appliedImpulseLateral1 = 0.f;
			m_pointCache[lastUsedIndex].m_appliedImpulseLateral2 = 0.f;
			m_pointCache[lastUsedIndex].m_lifeTime = 0;
		}

		Assert(m_pointCache[lastUsedIndex].m_userPersistentData == 0);
		m_cachedPoints--;

		if (gContactEndedCallback && m_cachedPoints == 0)
		{
			gContactEndedCallback(this);
		}
	}
	void replaceContactPoint(const ManifoldPoint& newPoint, i32 insertIndex)
	{
		Assert(validContactDistance(newPoint));

#define MAINTAIN_PERSISTENCY 1
#ifdef MAINTAIN_PERSISTENCY
		i32 lifeTime = m_pointCache[insertIndex].getLifeTime();
		Scalar appliedImpulse = m_pointCache[insertIndex].m_appliedImpulse;
		Scalar prevRHS = m_pointCache[insertIndex].m_prevRHS;
		Scalar appliedLateralImpulse1 = m_pointCache[insertIndex].m_appliedImpulseLateral1;
		Scalar appliedLateralImpulse2 = m_pointCache[insertIndex].m_appliedImpulseLateral2;

		bool replacePoint = true;
		///we keep existing contact points for friction anchors
		///if the friction force is within the Coulomb friction cone
		if (newPoint.m_contactPointFlags & DRX3D_CONTACT_FLAG_FRICTION_ANCHOR)
		{
			//   printf("appliedImpulse=%f\n", appliedImpulse);
			//   printf("appliedLateralImpulse1=%f\n", appliedLateralImpulse1);
			//   printf("appliedLateralImpulse2=%f\n", appliedLateralImpulse2);
			//   printf("mu = %f\n", m_pointCache[insertIndex].m_combinedFriction);
			Scalar mu = m_pointCache[insertIndex].m_combinedFriction;
			Scalar eps = 0;  //we could allow to enlarge or shrink the tolerance to check against the friction cone a bit, say 1e-7
			Scalar a = appliedLateralImpulse1 * appliedLateralImpulse1 + appliedLateralImpulse2 * appliedLateralImpulse2;
			Scalar b = eps + mu * appliedImpulse;
			b = b * b;
			replacePoint = (a) > (b);
		}

		if (replacePoint)
		{
			Assert(lifeTime >= 0);
			uk cache = m_pointCache[insertIndex].m_userPersistentData;

			m_pointCache[insertIndex] = newPoint;
			m_pointCache[insertIndex].m_userPersistentData = cache;
			m_pointCache[insertIndex].m_appliedImpulse = appliedImpulse;
			m_pointCache[insertIndex].m_prevRHS = prevRHS;
			m_pointCache[insertIndex].m_appliedImpulseLateral1 = appliedLateralImpulse1;
			m_pointCache[insertIndex].m_appliedImpulseLateral2 = appliedLateralImpulse2;
		}

		m_pointCache[insertIndex].m_lifeTime = lifeTime;
#else
		clearUserCache(m_pointCache[insertIndex]);
		m_pointCache[insertIndex] = newPoint;

#endif
	}

	bool validContactDistance(const ManifoldPoint& pt) const
	{
		return pt.m_distance1 <= getContactBreakingThreshold();
	}
	/// calculated new worldspace coordinates and depth, and reject points that exceed the collision margin
	void refreshContactPoints(const Transform2& trA, const Transform2& trB);

	SIMD_FORCE_INLINE void clearManifold()
	{
		i32 i;
		for (i = 0; i < m_cachedPoints; i++)
		{
			clearUserCache(m_pointCache[i]);
		}

		if (gContactEndedCallback && m_cachedPoints)
		{
			gContactEndedCallback(this);
		}
		m_cachedPoints = 0;
	}

	i32 calculateSerializeBufferSize() const;
	tukk serialize(const class PersistentManifold* manifold, uk dataBuffer, class Serializer* serializer) const;
	void deSerialize(const struct PersistentManifoldDoubleData* manifoldDataPtr);
	void deSerialize(const struct PersistentManifoldFloatData* manifoldDataPtr);
};

// clang-format off

struct PersistentManifoldDoubleData
{
	Vec3DoubleData m_pointCacheLocalPointA[4];
	Vec3DoubleData m_pointCacheLocalPointB[4];
	Vec3DoubleData m_pointCachePositionWorldOnA[4];
	Vec3DoubleData m_pointCachePositionWorldOnB[4];
	Vec3DoubleData m_pointCacheNormalWorldOnB[4];
	Vec3DoubleData	m_pointCacheLateralFrictionDir1[4];
	Vec3DoubleData	m_pointCacheLateralFrictionDir2[4];
	double m_pointCacheDistance[4];
	double m_pointCacheAppliedImpulse[4];
	double m_pointCachePrevRHS[4];
	 double m_pointCacheCombinedFriction[4];
	double m_pointCacheCombinedRollingFriction[4];
	double m_pointCacheCombinedSpinningFriction[4];
	double m_pointCacheCombinedRestitution[4];
	i32	m_pointCachePartId0[4];
	i32	m_pointCachePartId1[4];
	i32	m_pointCacheIndex0[4];
	i32	m_pointCacheIndex1[4];
	i32 m_pointCacheContactPointFlags[4];
	double m_pointCacheAppliedImpulseLateral1[4];
	double m_pointCacheAppliedImpulseLateral2[4];
	double m_pointCacheContactMotion1[4];
	double m_pointCacheContactMotion2[4];
	double m_pointCacheContactCFM[4];
	double m_pointCacheCombinedContactStiffness1[4];
	double m_pointCacheContactERP[4];
	double m_pointCacheCombinedContactDamping1[4];
	double m_pointCacheFrictionCFM[4];
	i32 m_pointCacheLifeTime[4];

	i32 m_numCachedPoints;
	i32 m_companionIdA;
	i32 m_companionIdB;
	i32 m_index1a;

	i32 m_objectType;
	double	m_contactBreakingThreshold;
	double	m_contactProcessingThreshold;
	i32 m_padding;

	CollisionObject2DoubleData *m_body0;
	CollisionObject2DoubleData *m_body1;
};


struct PersistentManifoldFloatData
{
	Vec3FloatData m_pointCacheLocalPointA[4];
	Vec3FloatData m_pointCacheLocalPointB[4];
	Vec3FloatData m_pointCachePositionWorldOnA[4];
	Vec3FloatData m_pointCachePositionWorldOnB[4];
	Vec3FloatData m_pointCacheNormalWorldOnB[4];
	Vec3FloatData	m_pointCacheLateralFrictionDir1[4];
	Vec3FloatData	m_pointCacheLateralFrictionDir2[4];
	float m_pointCacheDistance[4];
	float m_pointCacheAppliedImpulse[4];
	float m_pointCachePrevRHS[4];
	float m_pointCacheCombinedFriction[4];
	float m_pointCacheCombinedRollingFriction[4];
	float m_pointCacheCombinedSpinningFriction[4];
	float m_pointCacheCombinedRestitution[4];
	i32	m_pointCachePartId0[4];
	i32	m_pointCachePartId1[4];
	i32	m_pointCacheIndex0[4];
	i32	m_pointCacheIndex1[4];
	i32 m_pointCacheContactPointFlags[4];
	float m_pointCacheAppliedImpulseLateral1[4];
	float m_pointCacheAppliedImpulseLateral2[4];
	float m_pointCacheContactMotion1[4];
	float m_pointCacheContactMotion2[4];
	float m_pointCacheContactCFM[4];
	float m_pointCacheCombinedContactStiffness1[4];
	float m_pointCacheContactERP[4];
	float m_pointCacheCombinedContactDamping1[4];
	float m_pointCacheFrictionCFM[4];
	i32 m_pointCacheLifeTime[4];

	i32 m_numCachedPoints;
	i32 m_companionIdA;
	i32 m_companionIdB;
	i32 m_index1a;

	i32 m_objectType;
	float	m_contactBreakingThreshold;
	float	m_contactProcessingThreshold;
	i32 m_padding;

	CollisionObject2FloatData *m_body0;
	CollisionObject2FloatData *m_body1;
};

// clang-format on

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define PersistentManifoldData PersistentManifoldDoubleData
#define PersistentManifoldDataName "PersistentManifoldDoubleData"
#else
#define PersistentManifoldData PersistentManifoldFloatData
#define PersistentManifoldDataName "PersistentManifoldFloatData"
#endif  //DRX3D_USE_DOUBLE_PRECISION

#endif  //DRX3D_PERSISTENT_MANIFOLD_H
