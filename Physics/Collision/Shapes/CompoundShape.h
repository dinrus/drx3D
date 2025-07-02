#ifndef DRX3D_COMPOUND_SHAPE_H
#define DRX3D_COMPOUND_SHAPE_H

#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>

#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Matrix3x3.h>
#include <drx3D/Physics/Collision/Shapes/CollisionMargin.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

//class OptimizedBvh;
struct Dbvt;

ATTRIBUTE_ALIGNED16(struct)
CompoundShapeChild
{
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	Transform2 m_transform;
	CollisionShape* m_childShape;
	i32 m_childShapeType;
	Scalar m_childMargin;
	struct DbvtNode* m_node;
};

SIMD_FORCE_INLINE bool operator==(const CompoundShapeChild& c1, const CompoundShapeChild& c2)
{
	return (c1.m_transform == c2.m_transform &&
			c1.m_childShape == c2.m_childShape &&
			c1.m_childShapeType == c2.m_childShapeType &&
			c1.m_childMargin == c2.m_childMargin);
}

/// The CompoundShape allows to store multiple other Shapes
/// This allows for moving concave collision objects. This is more general then the static concave BvhTriangleMeshShape.
/// It has an (optional) dynamic aabb tree to accelerate early rejection tests.
/// @todo: This aabb tree can also be use to speed up ray tests on CompoundShape, see http://code.google.com/p/bullet/issues/detail?id=25
/// Currently, removal of child shapes is only supported when disabling the aabb tree (pass 'false' in the constructor of CompoundShape)
ATTRIBUTE_ALIGNED16(class)
CompoundShape : public CollisionShape
{
protected:
	AlignedObjectArray<CompoundShapeChild> m_children;
	Vec3 m_localAabbMin;
	Vec3 m_localAabbMax;

	Dbvt* m_dynamicAabbTree;

	///increment m_updateRevision when adding/removing/replacing child shapes, so that some caches can be updated
	i32 m_updateRevision;

	Scalar m_collisionMargin;

	Vec3 m_localScaling;

public:
	DRX3D_DECLARE_ALIGNED_ALLOCATOR();

	explicit CompoundShape(bool enableDynamicAabbTree = true, i32k initialChildCapacity = 0);

	virtual ~CompoundShape();

	void addChildShape(const Transform2& localTransform2, CollisionShape* shape);

	/// Remove all children shapes that contain the specified shape
	virtual void removeChildShape(CollisionShape * shape);

	void removeChildShapeByIndex(i32 childShapeindex);

	i32 getNumChildShapes() const
	{
		return i32(m_children.size());
	}

	CollisionShape* getChildShape(i32 index)
	{
		return m_children[index].m_childShape;
	}
	const CollisionShape* getChildShape(i32 index) const
	{
		return m_children[index].m_childShape;
	}

	Transform2& getChildTransform(i32 index)
	{
		return m_children[index].m_transform;
	}
	const Transform2& getChildTransform(i32 index) const
	{
		return m_children[index].m_transform;
	}

	///set a new transform for a child, and update internal data structures (local aabb and dynamic tree)
	void updateChildTransform(i32 childIndex, const Transform2& newChildTransform2, bool shouldRecalculateLocalAabb = true);

	CompoundShapeChild* getChildList()
	{
		return &m_children[0];
	}

	///getAabb's default implementation is brute force, expected derived classes to implement a fast dedicated version
	virtual void getAabb(const Transform2& t, Vec3& aabbMin, Vec3& aabbMax) const;

	/** Re-calculate the local Aabb. Is called at the end of removeChildShapes.
	Use this yourself if you modify the children or their transforms. */
	virtual void recalculateLocalAabb();

	virtual void setLocalScaling(const Vec3& scaling);

	virtual const Vec3& getLocalScaling() const
	{
		return m_localScaling;
	}

	virtual void calculateLocalInertia(Scalar mass, Vec3 & inertia) const;

	virtual void setMargin(Scalar margin)
	{
		m_collisionMargin = margin;
	}
	virtual Scalar getMargin() const
	{
		return m_collisionMargin;
	}
	virtual tukk getName() const
	{
		return "Compound";
	}

	const Dbvt* getDynamicAabbTree() const
	{
		return m_dynamicAabbTree;
	}

	Dbvt* getDynamicAabbTree()
	{
		return m_dynamicAabbTree;
	}

	void createAabbTreeFromChildren();

	///computes the exact moment of inertia and the transform from the coordinate system defined by the principal axes of the moment of inertia
	///and the center of mass to the current coordinate system. "masses" points to an array of masses of the children. The resulting transform
	///"principal" has to be applied inversely to all children transforms in order for the local coordinate system of the compound
	///shape to be centered at the center of mass and to coincide with the principal axes. This also necessitates a correction of the world transform
	///of the collision object by the principal transform.
	void calculatePrincipalAxisTransform(const Scalar* masses, Transform2& principal, Vec3& inertia) const;

	i32 getUpdateRevision() const
	{
		return m_updateRevision;
	}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;
};

// clang-format off

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct CompoundShapeChildData
{
	Transform2FloatData	m_transform;
	CollisionShapeData	*m_childShape;
	i32						m_childShapeType;
	float					m_childMargin;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct	CompoundShapeData
{
	CollisionShapeData		m_collisionShapeData;

	CompoundShapeChildData	*m_childShapePtr;

	i32							m_numChildShapes;

	float	m_collisionMargin;

};

// clang-format on

SIMD_FORCE_INLINE i32 CompoundShape::calculateSerializeBufferSize() const
{
	return sizeof(CompoundShapeData);
}

#endif  //DRX3D_COMPOUND_SHAPE_H
