#ifndef DRX3D_COLLISION_WORLD_IMPORTER_H
#define DRX3D_COLLISION_WORLD_IMPORTER_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>

class CollisionShape;
class CollisionObject2;
struct BulletSerializedArrays;

struct ConstraintInput;
class CollisionWorld;
struct CollisionShapeData;
class TriangleIndexVertexArray;
class StridingMeshInterface;
struct StridingMeshInterfaceData;
class GImpactMeshShape;
class OptimizedBvh;
struct TriangleInfoMap;
class BvhTriangleMeshShape;
class Point2PointConstraint;
class HingeConstraint;
class ConeTwistConstraint;
class Generic6DofConstraint;
class Generic6DofSpringConstraint;
class SliderConstraint;
class GearConstraint;
struct ContactSolverInfo;

class CollisionWorldImporter
{
protected:
	CollisionWorld* m_collisionWorld;

	i32 m_verboseMode;

	AlignedObjectArray<CollisionShape*> m_allocatedShapes;
	AlignedObjectArray<CollisionObject2*> m_allocatedRigidBodies;

	AlignedObjectArray<OptimizedBvh*> m_allocatedBvhs;
	AlignedObjectArray<TriangleInfoMap*> m_allocatedTriangleInfoMaps;
	AlignedObjectArray<TriangleIndexVertexArray*> m_allocatedTriangleIndexArrays;
	AlignedObjectArray<StridingMeshInterfaceData*> m_allocatedStridingMeshInterfaceDatas;
	AlignedObjectArray<CollisionObject2*> m_allocatedCollisionObjects;

	AlignedObjectArray<tuk> m_allocatedNames;

	AlignedObjectArray<i32*> m_indexArrays;
	AlignedObjectArray<i16*> m_shortIndexArrays;
	AlignedObjectArray<u8*> m_charIndexArrays;

	AlignedObjectArray<Vec3FloatData*> m_floatVertexArrays;
	AlignedObjectArray<Vec3DoubleData*> m_doubleVertexArrays;

	HashMap<HashPtr, OptimizedBvh*> m_bvhMap;
	HashMap<HashPtr, TriangleInfoMap*> m_timMap;

	HashMap<HashString, CollisionShape*> m_nameShapeMap;
	HashMap<HashString, CollisionObject2*> m_nameColObjMap;

	HashMap<HashPtr, tukk > m_objectNameMap;

	HashMap<HashPtr, CollisionShape*> m_shapeMap;
	HashMap<HashPtr, CollisionObject2*> m_bodyMap;

	//methods

	tuk duplicateName(tukk name);

	CollisionShape* convertCollisionShape(CollisionShapeData* shapeData);

public:
	CollisionWorldImporter(CollisionWorld* world);

	virtual ~CollisionWorldImporter();

	bool convertAllObjects(BulletSerializedArrays* arrays);

	///delete all memory collision shapes, rigid bodies, constraints etc. allocated during the load.
	///make sure you don't use the dynamics world containing objects after you call this method
	virtual void deleteAllData();

	void setVerboseMode(i32 verboseMode)
	{
		m_verboseMode = verboseMode;
	}

	i32 getVerboseMode() const
	{
		return m_verboseMode;
	}

	// query for data
	i32 getNumShapes() const;
	CollisionShape* getCollisionShapeByIndex(i32 index);
	i32 getNumRigidBodies() const;
	CollisionObject2* getRigidBodyByIndex(i32 index) const;

	i32 getNumBvhs() const;
	OptimizedBvh* getBvhByIndex(i32 index) const;
	i32 getNumTriangleInfoMaps() const;
	TriangleInfoMap* getTriangleInfoMapByIndex(i32 index) const;

	// queris involving named objects
	CollisionShape* getCollisionShapeByName(tukk name);
	CollisionObject2* getCollisionObjectByName(tukk name);

	tukk getNameForPointer(ukk ptr) const;

	///those virtuals are called by load and can be overridden by the user

	//bodies

	virtual CollisionObject2* createCollisionObject2(const Transform2& startTransform2, CollisionShape* shape, tukk bodyName);

	///shapes

	virtual CollisionShape* createPlaneShape(const Vec3& planeNormal, Scalar planeConstant);
	virtual CollisionShape* createBoxShape(const Vec3& halfExtents);
	virtual CollisionShape* createSphereShape(Scalar radius);
	virtual CollisionShape* createCapsuleShapeX(Scalar radius, Scalar height);
	virtual CollisionShape* createCapsuleShapeY(Scalar radius, Scalar height);
	virtual CollisionShape* createCapsuleShapeZ(Scalar radius, Scalar height);

	virtual CollisionShape* createCylinderShapeX(Scalar radius, Scalar height);
	virtual CollisionShape* createCylinderShapeY(Scalar radius, Scalar height);
	virtual CollisionShape* createCylinderShapeZ(Scalar radius, Scalar height);
	virtual CollisionShape* createConeShapeX(Scalar radius, Scalar height);
	virtual CollisionShape* createConeShapeY(Scalar radius, Scalar height);
	virtual CollisionShape* createConeShapeZ(Scalar radius, Scalar height);
	virtual class TriangleIndexVertexArray* createTriangleMeshContainer();
	virtual BvhTriangleMeshShape* createBvhTriangleMeshShape(StridingMeshInterface* trimesh, OptimizedBvh* bvh);
	virtual CollisionShape* createConvexTriangleMeshShape(StridingMeshInterface* trimesh);
#ifdef SUPPORT_GIMPACT_SHAPE_IMPORT
	virtual GImpactMeshShape* createGimpactShape(StridingMeshInterface* trimesh);
#endif  //SUPPORT_GIMPACT_SHAPE_IMPORT
	virtual StridingMeshInterfaceData* createStridingMeshInterfaceData(StridingMeshInterfaceData* interfaceData);

	virtual class ConvexHullShape* createConvexHullShape();
	virtual class CompoundShape* createCompoundShape();
	virtual class ScaledBvhTriangleMeshShape* createScaledTrangleMeshShape(BvhTriangleMeshShape* meshShape, const Vec3& localScalingBvhTriangleMeshShape);

	virtual class MultiSphereShape* createMultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres);

	virtual TriangleIndexVertexArray* createMeshInterface(StridingMeshInterfaceData& meshData);

	///acceleration and connectivity structures
	virtual OptimizedBvh* createOptimizedBvh();
	virtual TriangleInfoMap* createTriangleInfoMap();
};

#endif  //DRX3D_WORLD_IMPORTER_H
