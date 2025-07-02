#ifndef DRX3D_CONVEX_INTERNAL_SHAPE_H
#define DRX3D_CONVEX_INTERNAL_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/ConvexShape.h>
#include <drx3D/Maths/Linear/AabbUtil2.h>

///The ConvexInternalShape is an internal base class, shared by most convex shape implementations.
///The ConvexInternalShape uses a default collision margin set to CONVEX_DISTANCE_MARGIN.
///This collision margin used by Gjk and some other algorithms, see also CollisionMargin.h
///Note that when creating small shapes (derived from ConvexInternalShape),
///you need to make sure to set a smaller collision margin, using the 'setMargin' API
///There is a automatic mechanism 'setSafeMargin' used by BoxShape and CylinderShape
ATTRIBUTE_ALIGNED16(class)
ConvexInternalShape : public ConvexShape
{
protected:
	//local scaling. collisionMargin is not scaled !
	Vec3 m_localScaling;

	Vec3 m_implicitShapeDimensions;

	Scalar m_collisionMargin;

	Scalar m_padding;

	ConvexInternalShape();

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	virtual ~ConvexInternalShape()
	{
	}

	virtual Vec3 localGetSupportingVertex(const Vec3& vec) const;

	const Vec3& getImplicitShapeDimensions() const
	{
		return m_implicitShapeDimensions;
	}

	///warning: use setImplicitShapeDimensions with care
	///changing a collision shape while the body is in the world is not recommended,
	///it is best to remove the body from the world, then make the change, and re-add it
	///alternatively flush the contact points, see documentation for 'cleanProxyFromPairs'
	void setImplicitShapeDimensions(const Vec3& dimensions)
	{
		m_implicitShapeDimensions = dimensions;
	}

	void setSafeMargin(Scalar minDimension, Scalar defaultMarginMultiplier = 0.1f)
	{
		Scalar safeMargin = defaultMarginMultiplier * minDimension;
		if (safeMargin < getMargin())
		{
			setMargin(safeMargin);
		}
	}
	void setSafeMargin(const Vec3& halfExtents, Scalar defaultMarginMultiplier = 0.1f)
	{
		//see http://code.google.com/p/bullet/issues/detail?id=349
		//this margin check could could be added to other collision shapes too,
		//or add some assert/warning somewhere
		Scalar minDimension = halfExtents[halfExtents.minAxis()];
		setSafeMargin(minDimension, defaultMarginMultiplier);
	}

	///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
	void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const
	{
		getAabbSlow(t, aabbMin, aabbMax);
	}

	virtual void getAabbSlow(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	virtual void setLocalScaling(const Vec3& scaling);
	virtual const Vec3& getLocalScaling() const
	{
		return m_localScaling;
	}

	const Vec3& getLocalScalingNV() const
	{
		return m_localScaling;
	}

	virtual void setMargin(Scalar margin)
	{
		m_collisionMargin = margin;
	}
	virtual Scalar getMargin() const
	{
		return m_collisionMargin;
	}

	Scalar getMarginNV() const
	{
		return m_collisionMargin;
	}

	virtual i32 getNumPreferredPenetrationDirections() const
	{
		return 0;
	}

	virtual void getPreferredPenetrationDirection(i32 index, Vec3& penetrationVector) const
	{
		(void)penetrationVector;
		(void)index;
		Assert(0);
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct ConvexInternalShapeData
{
	CollisionShapeData m_collisionShapeData;

	Vec3FloatData m_localScaling;

	Vec3FloatData m_implicitShapeDimensions;

	float m_collisionMargin;

	i32 m_padding;
};

SIMD_FORCE_INLINE i32 ConvexInternalShape::calculateSerializeBufferSize() const
{
	return sizeof(ConvexInternalShapeData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk ConvexInternalShape::serialize(uk dataBuffer, Serializer* serializer) const
{
	ConvexInternalShapeData* shapeData = (ConvexInternalShapeData*)dataBuffer;
	CollisionShape::serialize(&shapeData->m_collisionShapeData, serializer);

	m_implicitShapeDimensions.serializeFloat(shapeData->m_implicitShapeDimensions);
	m_localScaling.serializeFloat(shapeData->m_localScaling);
	shapeData->m_collisionMargin = float(m_collisionMargin);

	// Fill padding with zeros to appease msan.
	shapeData->m_padding = 0;

	return "ConvexInternalShapeData";
}

//ConvexInternalAabbCachingShape adds local aabb caching for convex shapes, to avoid expensive bounding box calculations
class ConvexInternalAabbCachingShape : public ConvexInternalShape
{
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;
	bool m_isLocalAabbValid;

protected:
	ConvexInternalAabbCachingShape();

	void setCachedLocalAabb(const Vec3& aabbMin, const Vec3& aabbMax)
	{
		m_isLocalAabbValid = true;
		m_localAabbMin = aabbMin;
		m_localAabbMax = aabbMax;
	}

	inline void getCachedLocalAabb(Vec3& aabbMin, Vec3& aabbMax) const
	{
		Assert(m_isLocalAabbValid);
		aabbMin = m_localAabbMin;
		aabbMax = m_localAabbMax;
	}

	inline void getNonvirtualAabb(const Transform2& trans, Vec3& aabbMin, Vec3& aabbMax, Scalar margin) const
	{
		//lazy evaluation of local aabb
		Assert(m_isLocalAabbValid);
		Transform2Aabb(m_localAabbMin, m_localAabbMax, margin, trans, aabbMin, aabbMax);
	}

public:
	virtual void setLocalScaling(const Vec3& scaling);

	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	void recalcLocalAabb();
};

#endif  //DRX3D_CONVEX_INTERNAL_SHAPE_H
