#ifndef DRX3D_WORLD_IMPORTER_H
#define DRX3D_WORLD_IMPORTER_H

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/HashMap.h>

class CollisionShape;
class CollisionObject2;
class RigidBody;
class TypedConstraint;
class DynamicsWorld;
struct ConstraintInput;
class RigidBodyColladaInfo;
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
class Generic6DofSpring2Constraint;
class SliderConstraint;
class GearConstraint;
struct ContactSolverInfo;
struct TypedConstraintData;
struct TypedConstraintFloatData;
struct TypedConstraintDoubleData;

struct RigidBodyDoubleData;
struct RigidBodyFloatData;

#ifdef DRX3D_USE_DOUBLE_PRECISION
#define RigidBodyData RigidBodyDoubleData
#else
#define RigidBodyData RigidBodyFloatData
#endif  //DRX3D_USE_DOUBLE_PRECISION

enum WorldImporterFlags
{
	eRESTORE_EXISTING_OBJECTS = 1,  //don't create new objects
};

class WorldImporter
{
protected:
	DynamicsWorld* m_dynamicsWorld;

	i32 m_verboseMode;
	i32 m_importerFlags;

	AlignedObjectArray<CollisionShape*> m_allocatedCollisionShapes;
	AlignedObjectArray<CollisionObject2*> m_allocatedRigidBodies;
	AlignedObjectArray<TypedConstraint*> m_allocatedConstraints;
	AlignedObjectArray<OptimizedBvh*> m_allocatedBvhs;
	AlignedObjectArray<TriangleInfoMap*> m_allocatedTriangleInfoMaps;
	AlignedObjectArray<TriangleIndexVertexArray*> m_allocatedTriangleIndexArrays;
	AlignedObjectArray<StridingMeshInterfaceData*> m_allocatedbtStridingMeshInterfaceDatas;

	AlignedObjectArray<tuk> m_allocatedNames;

	AlignedObjectArray<i32*> m_indexArrays;
	AlignedObjectArray<i16*> m_shortIndexArrays;
	AlignedObjectArray<u8*> m_charIndexArrays;

    AlignedObjectArray<Vec3FloatData*> m_floatVertexArrays;
	AlignedObjectArray<Vec3DoubleData*> m_doubleVertexArrays;

	HashMap<HashPtr, OptimizedBvh*> m_bvhMap;
	HashMap<HashPtr, TriangleInfoMap*> m_timMap;

	HashMap<HashString, CollisionShape*> m_nameShapeMap;
	HashMap<HashString, RigidBody*> m_nameBodyMap;
	HashMap<HashString, TypedConstraint*> m_nameConstraintMap;
	HashMap<HashPtr, tukk > m_objectNameMap;

	HashMap<HashPtr, CollisionShape*> m_shapeMap;
	HashMap<HashPtr, CollisionObject2*> m_bodyMap;

	//methods

	static RigidBody& getFixedBody();

	tuk duplicateName(tukk name);

	CollisionShape* convertCollisionShape(CollisionShapeData* shapeData);

	void convertConstraintBackwardsCompatible281(TypedConstraintData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion);
	void convertConstraintFloat(TypedConstraintFloatData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion);
	void convertConstraintDouble(TypedConstraintDoubleData* constraintData, RigidBody* rbA, RigidBody* rbB, i32 fileVersion);
	void convertRigidBodyFloat(RigidBodyFloatData* colObjData);
	void convertRigidBodyDouble(RigidBodyDoubleData* colObjData);

public:
	WorldImporter(DynamicsWorld* world);

	virtual ~WorldImporter();

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

	void setImporterFlags(i32 importerFlags)
	{
		m_importerFlags = importerFlags;
	}

	i32 getImporterFlags() const
	{
		return m_importerFlags;
	}

	// query for data
	i32 getNumCollisionShapes() const;
	CollisionShape* getCollisionShapeByIndex(i32 index);
	i32 getNumRigidBodies() const;
	CollisionObject2* getRigidBodyByIndex(i32 index) const;
	i32 getNumConstraints() const;
	TypedConstraint* getConstraintByIndex(i32 index) const;
	i32 getNumBvhs() const;
	OptimizedBvh* getBvhByIndex(i32 index) const;
	i32 getNumTriangleInfoMaps() const;
	TriangleInfoMap* getTriangleInfoMapByIndex(i32 index) const;

	// queris involving named objects
	CollisionShape* getCollisionShapeByName(tukk name);
	RigidBody* getRigidBodyByName(tukk name);
	TypedConstraint* getConstraintByName(tukk name);
	tukk getNameForPointer(ukk ptr) const;

	///those virtuals are called by load and can be overridden by the user

	virtual void setDynamicsWorldInfo(const Vec3& gravity, const ContactSolverInfo& solverInfo);

	//bodies
	virtual RigidBody* createRigidBody(bool isDynamic, Scalar mass, const Transform2& startTransform, CollisionShape* shape, tukk bodyName);
	virtual CollisionObject2* createCollisionObject(const Transform2& startTransform, CollisionShape* shape, tukk bodyName);

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
	virtual GImpactMeshShape* createGimpactShape(StridingMeshInterface* trimesh);
	virtual StridingMeshInterfaceData* createStridingMeshInterfaceData(StridingMeshInterfaceData* interfaceData);

	virtual class ConvexHullShape* createConvexHullShape();
	virtual class CompoundShape* createCompoundShape();
	virtual class ScaledBvhTriangleMeshShape* createScaledTrangleMeshShape(BvhTriangleMeshShape* meshShape, const Vec3& localScalingbtBvhTriangleMeshShape);

	virtual class MultiSphereShape* createMultiSphereShape(const Vec3* positions, const Scalar* radi, i32 numSpheres);

	virtual TriangleIndexVertexArray* createMeshInterface(StridingMeshInterfaceData& meshData);

	virtual class HeightfieldTerrainShape* createHeightfieldShape(i32 heightStickWidth, i32 heightStickLength,
		ukk heightfieldData, Scalar heightScale,
		Scalar minHeight, Scalar maxHeight,
		i32 upAxis, i32 heightDataType,
		bool flipQuadEdges);

	///acceleration and connectivity structures
	virtual OptimizedBvh* createOptimizedBvh();
	virtual TriangleInfoMap* createTriangleInfoMap();

	///constraints
	virtual Point2PointConstraint* createPoint2PointConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& pivotInA, const Vec3& pivotInB);
	virtual Point2PointConstraint* createPoint2PointConstraint(RigidBody& rbA, const Vec3& pivotInA);
	virtual HingeConstraint* createHingeConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& rbAFrame, const Transform2& rbBFrame, bool useReferenceFrameA = false);
	virtual HingeConstraint* createHingeConstraint(RigidBody& rbA, const Transform2& rbAFrame, bool useReferenceFrameA = false);
	virtual ConeTwistConstraint* createConeTwistConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& rbAFrame, const Transform2& rbBFrame);
	virtual ConeTwistConstraint* createConeTwistConstraint(RigidBody& rbA, const Transform2& rbAFrame);
	virtual Generic6DofConstraint* createGeneric6DofConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	virtual Generic6DofConstraint* createGeneric6DofConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameB);
	virtual Generic6DofSpringConstraint* createGeneric6DofSpringConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	virtual Generic6DofSpring2Constraint* createGeneric6DofSpring2Constraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, i32 rotateOrder);

	virtual SliderConstraint* createSliderConstraint(RigidBody& rbA, RigidBody& rbB, const Transform2& frameInA, const Transform2& frameInB, bool useLinearReferenceFrameA);
	virtual SliderConstraint* createSliderConstraint(RigidBody& rbB, const Transform2& frameInB, bool useLinearReferenceFrameA);
	virtual GearConstraint* createGearConstraint(RigidBody& rbA, RigidBody& rbB, const Vec3& axisInA, const Vec3& axisInB, Scalar ratio);
};

#endif  //DRX3D_WORLD_IMPORTER_H