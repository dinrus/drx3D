#ifndef DRX3D_COLLISION_OBJECT_WRAPPER_H
#define DRX3D_COLLISION_OBJECT_WRAPPER_H

//CollisionObject2Wrapperis an internal data structure.
///Most users can ignore this and use CollisionObject2 and CollisionShape instead
class CollisionShape;
class CollisionObject2;
class Transform2;
#include <drx3D/Maths/Linear/Scalar.h>  // for SIMD_FORCE_INLINE definition

#define DRX3D_DECLARE_STACK_ONLY_OBJECT \
private:                             \
	uk operator new(size_t size); \
	void operator delete(uk );

struct CollisionObject2Wrapper;
struct CollisionObject2Wrapper
{
	DRX3D_DECLARE_STACK_ONLY_OBJECT

private:
	CollisionObject2Wrapper(const CollisionObject2Wrapper&);  // not implemented. Not allowed.
	CollisionObject2Wrapper* operator=(const CollisionObject2Wrapper&);

public:
	const CollisionObject2Wrapper* m_parent;
	const CollisionShape* m_shape;
	const CollisionObject2* m_collisionObject;
	const Transform2& m_worldTransform;
    const Transform2* m_preTransform2;
	i32 m_partId;
	i32 m_index;

	CollisionObject2Wrapper(const CollisionObject2Wrapper* parent, const CollisionShape* shape, const CollisionObject2* collisionObject, const Transform2& worldTransform2, i32 partId, i32 index)
		: m_parent(parent), m_shape(shape), m_collisionObject(collisionObject), m_worldTransform(worldTransform2), m_preTransform2(nullptr), m_partId(partId), m_index(index)
	{
	}
    
    CollisionObject2Wrapper(const CollisionObject2Wrapper* parent, const CollisionShape* shape, const CollisionObject2* collisionObject, const Transform2& worldTransform2, const Transform2& preTransform2, i32 partId, i32 index)
    : m_parent(parent), m_shape(shape), m_collisionObject(collisionObject), m_worldTransform(worldTransform2), m_preTransform2(&preTransform2), m_partId(partId), m_index(index)
    {
    }

	SIMD_FORCE_INLINE const Transform2& getWorldTransform() const { return m_worldTransform; }
	SIMD_FORCE_INLINE const CollisionObject2* getCollisionObject() const { return m_collisionObject; }
	SIMD_FORCE_INLINE const CollisionShape* getCollisionShape() const { return m_shape; }
};

#endif  //DRX3D_COLLISION_OBJECT_WRAPPER_H
