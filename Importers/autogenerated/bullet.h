// Auto generated from drx3D/Extras/HeaderGenerator/bulletGenerate.py
#ifndef __DRX3D_H__
#define __DRX3D_H__
{

// put an empty struct in the case
typedef struct bInvalidHandle {
	i32 unused;
}bInvalidHandle;

    class PointerArray;
    class PhysicsSystem;
    class ListBase;
    class Vec3FloatData;
    class Vec3DoubleData;
    class QuatFloatData;
    class QuatDoubleData;
    class Matrix3x3FloatData;
    class Matrix3x3DoubleData;
    class Transform2FloatData;
    class Transform2DoubleData;
    class BvhSureeInfoData;
    class OptimizedBvhNodeFloatData;
    class OptimizedBvhNodeDoubleData;
    class QuantizedBvhNodeData;
    class QuantizedBvhFloatData;
    class QuantizedBvhDoubleData;
    class CollisionShapeData;
    class StaticPlaneShapeData;
    class ConvexInternalShapeData;
    class PositionAndRadius;
    class MultiSphereShapeData;
    class IntIndexData;
    class ShortIntIndexData;
    class ShortIntIndexTripletData;
    class CharIndexTripletData;
    class MeshPartData;
    class StridingMeshInterfaceData;
    class TriangleMeshShapeData;
    class ScaledTriangleMeshShapeData;
    class CompoundShapeChildData;
    class CompoundShapeData;
    class CylinderShapeData;
    class ConeShapeData;
    class CapsuleShapeData;
    class TriangleInfoData;
    class TriangleInfoMapData;
    class PersistentManifoldDoubleData;
    class PersistentManifoldFloatData;
    class GImpactMeshShapeData;
    class ConvexHullShapeData;
    class CollisionObject2DoubleData;
    class CollisionObject2FloatData;
    class ContactSolverInfoDoubleData;
    class ContactSolverInfoFloatData;
    class DynamicsWorldDoubleData;
    class DynamicsWorldFloatData;
    class RigidBodyFloatData;
    class RigidBodyDoubleData;
    class ConstraintInfo1;
    class TypedConstraintFloatData;
    class TypedConstraintData;
    class TypedConstraintDoubleData;
    class Point2PointConstraintFloatData;
    class Point2PointConstraintDoubleData2;
    class Point2PointConstraintDoubleData;
    class HingeConstraintDoubleData;
    class HingeConstraintFloatData;
    class HingeConstraintDoubleData2;
    class ConeTwistConstraintDoubleData;
    class ConeTwistConstraintData;
    class Generic6DofConstraintData;
    class Generic6DofConstraintDoubleData2;
    class Generic6DofSpringConstraintData;
    class Generic6DofSpringConstraintDoubleData2;
    class Generic6DofSpring2ConstraintData;
    class Generic6DofSpring2ConstraintDoubleData2;
    class SliderConstraintData;
    class SliderConstraintDoubleData;
    class GearConstraintFloatData;
    class GearConstraintDoubleData;
    class SoftBodyMaterialData;
    class SoftBodyNodeData;
    class SoftBodyLinkData;
    class SoftBodyFaceData;
    class SoftBodyTetraData;
    class SoftRigidAnchorData;
    class SoftBodyConfigData;
    class SoftBodyPoseData;
    class SoftBodyClusterData;
    class SoftBodyJointData;
    class SoftBodyFloatData;
    class MultiBodyLinkDoubleData;
    class MultiBodyLinkFloatData;
    class MultiBodyDoubleData;
    class MultiBodyFloatData;
    class MultiBodyLinkColliderFloatData;
    class MultiBodyLinkColliderDoubleData;
// -------------------------------------------------- //
    class PointerArray
    {
    public:
        i32 m_size;
        i32 m_capacity;
        uk m_data;
    };


// -------------------------------------------------- //
    class PhysicsSystem
    {
    public:
        PointerArray m_collisionShapes;
        PointerArray m_collisionObjects;
        PointerArray m_constraints;
    };


// -------------------------------------------------- //
    class ListBase
    {
    public:
        uk first;
        uk last;
    };


// -------------------------------------------------- //
    class Vec3FloatData
    {
    public:
        float m_floats[4];
    };


// -------------------------------------------------- //
    class Vec3DoubleData
    {
    public:
        double m_floats[4];
    };


// -------------------------------------------------- //
    class QuatFloatData
    {
    public:
        float m_floats[4];
    };


// -------------------------------------------------- //
    class QuatDoubleData
    {
    public:
        double m_floats[4];
    };


// -------------------------------------------------- //
    class Matrix3x3FloatData
    {
    public:
        Vec3FloatData m_el[3];
    };


// -------------------------------------------------- //
    class Matrix3x3DoubleData
    {
    public:
        Vec3DoubleData m_el[3];
    };


// -------------------------------------------------- //
    class Transform2FloatData
    {
    public:
        Matrix3x3FloatData m_basis;
        Vec3FloatData m_origin;
    };


// -------------------------------------------------- //
    class Transform2DoubleData
    {
    public:
        Matrix3x3DoubleData m_basis;
        Vec3DoubleData m_origin;
    };


// -------------------------------------------------- //
    class BvhSureeInfoData
    {
    public:
        i32 m_rootNodeIndex;
        i32 m_sureeSize;
        short m_quantizedAabbMin[3];
        short m_quantizedAabbMax[3];
    };


// -------------------------------------------------- //
    class OptimizedBvhNodeFloatData
    {
    public:
        Vec3FloatData m_aabbMinOrg;
        Vec3FloatData m_aabbMaxOrg;
        i32 m_escapeIndex;
        i32 m_subPart;
        i32 m_triangleIndex;
        char m_pad[4];
    };


// -------------------------------------------------- //
    class OptimizedBvhNodeDoubleData
    {
    public:
        Vec3DoubleData m_aabbMinOrg;
        Vec3DoubleData m_aabbMaxOrg;
        i32 m_escapeIndex;
        i32 m_subPart;
        i32 m_triangleIndex;
        char m_pad[4];
    };


// -------------------------------------------------- //
    class QuantizedBvhNodeData
    {
    public:
        short m_quantizedAabbMin[3];
        short m_quantizedAabbMax[3];
        i32 m_escapeIndexOrTriangleIndex;
    };


// -------------------------------------------------- //
    class QuantizedBvhFloatData
    {
    public:
        Vec3FloatData m_bvhAabbMin;
        Vec3FloatData m_bvhAabbMax;
        Vec3FloatData m_bvhQuantization;
        i32 m_curNodeIndex;
        i32 m_useQuantization;
        i32 m_numContiguousLeafNodes;
        i32 m_numQuantizedContiguousNodes;
        OptimizedBvhNodeFloatData *m_contiguousNodesPtr;
        QuantizedBvhNodeData *m_quantizedContiguousNodesPtr;
        BvhSureeInfoData *m_subTreeInfoPtr;
        i32 m_traversalMode;
        i32 m_numSureeHeaders;
    };


// -------------------------------------------------- //
    class QuantizedBvhDoubleData
    {
    public:
        Vec3DoubleData m_bvhAabbMin;
        Vec3DoubleData m_bvhAabbMax;
        Vec3DoubleData m_bvhQuantization;
        i32 m_curNodeIndex;
        i32 m_useQuantization;
        i32 m_numContiguousLeafNodes;
        i32 m_numQuantizedContiguousNodes;
        OptimizedBvhNodeDoubleData *m_contiguousNodesPtr;
        QuantizedBvhNodeData *m_quantizedContiguousNodesPtr;
        i32 m_traversalMode;
        i32 m_numSureeHeaders;
        BvhSureeInfoData *m_subTreeInfoPtr;
    };


// -------------------------------------------------- //
    class CollisionShapeData
    {
    public:
        char *m_name;
        i32 m_shapeType;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class StaticPlaneShapeData
    {
    public:
        CollisionShapeData m_collisionShapeData;
        Vec3FloatData m_localScaling;
        Vec3FloatData m_planeNormal;
        float m_planeConstant;
        char m_pad[4];
    };


// -------------------------------------------------- //
    class ConvexInternalShapeData
    {
    public:
        CollisionShapeData m_collisionShapeData;
        Vec3FloatData m_localScaling;
        Vec3FloatData m_implicitShapeDimensions;
        float m_collisionMargin;
        i32 m_padding;
    };


// -------------------------------------------------- //
    class PositionAndRadius
    {
    public:
        Vec3FloatData m_pos;
        float m_radius;
    };


// -------------------------------------------------- //
    class MultiSphereShapeData
    {
    public:
        ConvexInternalShapeData m_convexInternalShapeData;
        PositionAndRadius *m_localPositionArrayPtr;
        i32 m_localPositionArraySize;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class IntIndexData
    {
    public:
        i32 m_value;
    };


// -------------------------------------------------- //
    class ShortIntIndexData
    {
    public:
        short m_value;
        char m_pad[2];
    };


// -------------------------------------------------- //
    class ShortIntIndexTripletData
    {
    public:
        short m_values[3];
        char m_pad[2];
    };


// -------------------------------------------------- //
    class CharIndexTripletData
    {
    public:
        char m_values[3];
        char m_pad;
    };


// -------------------------------------------------- //
    class MeshPartData
    {
    public:
        Vec3FloatData *m_vertices3f;
        Vec3DoubleData *m_vertices3d;
        IntIndexData *m_indices32;
        ShortIntIndexTripletData *m_3indices16;
        CharIndexTripletData *m_3indices8;
        ShortIntIndexData *m_indices16;
        i32 m_numTriangles;
        i32 m_numVertices;
    };


// -------------------------------------------------- //
    class StridingMeshInterfaceData
    {
    public:
        MeshPartData *m_meshPartsPtr;
        Vec3FloatData m_scaling;
        i32 m_numMeshParts;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class TriangleMeshShapeData
    {
    public:
        CollisionShapeData m_collisionShapeData;
        StridingMeshInterfaceData m_meshInterface;
        QuantizedBvhFloatData *m_quantizedFloatBvh;
        QuantizedBvhDoubleData *m_quantizedDoubleBvh;
        TriangleInfoMapData *m_triangleInfoMap;
        float m_collisionMargin;
        char m_pad3[4];
    };


// -------------------------------------------------- //
    class ScaledTriangleMeshShapeData
    {
    public:
        TriangleMeshShapeData m_trimeshShapeData;
        Vec3FloatData m_localScaling;
    };


// -------------------------------------------------- //
    class CompoundShapeChildData
    {
    public:
        Transform2FloatData m_transform;
        CollisionShapeData *m_childShape;
        i32 m_childShapeType;
        float m_childMargin;
    };


// -------------------------------------------------- //
    class CompoundShapeData
    {
    public:
        CollisionShapeData m_collisionShapeData;
        CompoundShapeChildData *m_childShapePtr;
        i32 m_numChildShapes;
        float m_collisionMargin;
    };


// -------------------------------------------------- //
    class CylinderShapeData
    {
    public:
        ConvexInternalShapeData m_convexInternalShapeData;
        i32 m_upAxis;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class ConeShapeData
    {
    public:
        ConvexInternalShapeData m_convexInternalShapeData;
        i32 m_upIndex;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class CapsuleShapeData
    {
    public:
        ConvexInternalShapeData m_convexInternalShapeData;
        i32 m_upAxis;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class TriangleInfoData
    {
    public:
        i32 m_flags;
        float m_edgeV0V1Angle;
        float m_edgeV1V2Angle;
        float m_edgeV2V0Angle;
    };


// -------------------------------------------------- //
    class TriangleInfoMapData
    {
    public:
        i32 *m_hashTablePtr;
        i32 *m_nextPtr;
        TriangleInfoData *m_valueArrayPtr;
        i32 *m_keyArrayPtr;
        float m_convexEpsilon;
        float m_planarEpsilon;
        float m_equalVertexThreshold;
        float m_edgeDistanceThreshold;
        float m_zeroAreaThreshold;
        i32 m_nextSize;
        i32 m_hashTableSize;
        i32 m_numValues;
        i32 m_numKeys;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class PersistentManifoldDoubleData
    {
    public:
        Vec3DoubleData m_pointCacheLocalPointA[4];
        Vec3DoubleData m_pointCacheLocalPointB[4];
        Vec3DoubleData m_pointCachePositionWorldOnA[4];
        Vec3DoubleData m_pointCachePositionWorldOnB[4];
        Vec3DoubleData m_pointCacheNormalWorldOnB[4];
        Vec3DoubleData m_pointCacheLateralFrictionDir1[4];
        Vec3DoubleData m_pointCacheLateralFrictionDir2[4];
        double m_pointCacheDistance[4];
        double m_pointCacheAppliedImpulse[4];
        double m_pointCachePrevRHS[4];
        double m_pointCacheCombinedFriction[4];
        double m_pointCacheCombinedRollingFriction[4];
        double m_pointCacheCombinedSpinningFriction[4];
        double m_pointCacheCombinedRestitution[4];
        i32 m_pointCachePartId0[4];
        i32 m_pointCachePartId1[4];
        i32 m_pointCacheIndex0[4];
        i32 m_pointCacheIndex1[4];
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
        double m_contactBreakingThreshold;
        double m_contactProcessingThreshold;
        i32 m_padding;
        CollisionObject2DoubleData *m_body0;
        CollisionObject2DoubleData *m_body1;
    };


// -------------------------------------------------- //
    class PersistentManifoldFloatData
    {
    public:
        Vec3FloatData m_pointCacheLocalPointA[4];
        Vec3FloatData m_pointCacheLocalPointB[4];
        Vec3FloatData m_pointCachePositionWorldOnA[4];
        Vec3FloatData m_pointCachePositionWorldOnB[4];
        Vec3FloatData m_pointCacheNormalWorldOnB[4];
        Vec3FloatData m_pointCacheLateralFrictionDir1[4];
        Vec3FloatData m_pointCacheLateralFrictionDir2[4];
        float m_pointCacheDistance[4];
        float m_pointCacheAppliedImpulse[4];
        float m_pointCachePrevRHS[4];
        float m_pointCacheCombinedFriction[4];
        float m_pointCacheCombinedRollingFriction[4];
        float m_pointCacheCombinedSpinningFriction[4];
        float m_pointCacheCombinedRestitution[4];
        i32 m_pointCachePartId0[4];
        i32 m_pointCachePartId1[4];
        i32 m_pointCacheIndex0[4];
        i32 m_pointCacheIndex1[4];
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
        float m_contactBreakingThreshold;
        float m_contactProcessingThreshold;
        i32 m_padding;
        CollisionObject2FloatData *m_body0;
        CollisionObject2FloatData *m_body1;
    };


// -------------------------------------------------- //
    class GImpactMeshShapeData
    {
    public:
        CollisionShapeData m_collisionShapeData;
        StridingMeshInterfaceData m_meshInterface;
        Vec3FloatData m_localScaling;
        float m_collisionMargin;
        i32 m_gimpactSubType;
    };


// -------------------------------------------------- //
    class ConvexHullShapeData
    {
    public:
        ConvexInternalShapeData m_convexInternalShapeData;
        Vec3FloatData *m_unscaledPointsFloatPtr;
        Vec3DoubleData *m_unscaledPointsDoublePtr;
        i32 m_numUnscaledPoints;
        char m_padding3[4];
    };


// -------------------------------------------------- //
    class CollisionObject2DoubleData
    {
    public:
        uk m_broadphaseHandle;
        uk m_collisionShape;
        CollisionShapeData *m_rootCollisionShape;
        char *m_name;
        Transform2DoubleData m_worldTransform;
        Transform2DoubleData m_interpolationWorldTransform;
        Vec3DoubleData m_interpolationLinearVelocity;
        Vec3DoubleData m_interpolationAngularVelocity;
        Vec3DoubleData m_anisotropicFriction;
        double m_contactProcessingThreshold;
        double m_deactivationTime;
        double m_friction;
        double m_rollingFriction;
        double m_contactDamping;
        double m_contactStiffness;
        double m_restitution;
        double m_hitFraction;
        double m_ccdSweptSphereRadius;
        double m_ccdMotionThreshold;
        i32 m_hasAnisotropicFriction;
        i32 m_collisionFlags;
        i32 m_islandTag1;
        i32 m_companionId;
        i32 m_activationState1;
        i32 m_internalType;
        i32 m_checkCollideWith;
        i32 m_collisionFilterGroup;
        i32 m_collisionFilterMask;
        i32 m_uniqueId;
    };


// -------------------------------------------------- //
    class CollisionObject2FloatData
    {
    public:
        uk m_broadphaseHandle;
        uk m_collisionShape;
        CollisionShapeData *m_rootCollisionShape;
        char *m_name;
        Transform2FloatData m_worldTransform;
        Transform2FloatData m_interpolationWorldTransform;
        Vec3FloatData m_interpolationLinearVelocity;
        Vec3FloatData m_interpolationAngularVelocity;
        Vec3FloatData m_anisotropicFriction;
        float m_contactProcessingThreshold;
        float m_deactivationTime;
        float m_friction;
        float m_rollingFriction;
        float m_contactDamping;
        float m_contactStiffness;
        float m_restitution;
        float m_hitFraction;
        float m_ccdSweptSphereRadius;
        float m_ccdMotionThreshold;
        i32 m_hasAnisotropicFriction;
        i32 m_collisionFlags;
        i32 m_islandTag1;
        i32 m_companionId;
        i32 m_activationState1;
        i32 m_internalType;
        i32 m_checkCollideWith;
        i32 m_collisionFilterGroup;
        i32 m_collisionFilterMask;
        i32 m_uniqueId;
    };


// -------------------------------------------------- //
    class ContactSolverInfoDoubleData
    {
    public:
        double m_tau;
        double m_damping;
        double m_friction;
        double m_timeStep;
        double m_restitution;
        double m_maxErrorReduction;
        double m_sor;
        double m_erp;
        double m_erp2;
        double m_globalCfm;
        double m_splitImpulsePenetrationThreshold;
        double m_splitImpulseTurnErp;
        double m_linearSlop;
        double m_warmstartingFactor;
        double m_articulatedWarmstartingFactor;
        double m_maxGyroscopicForce;
        double m_singleAxisRollingFrictionThreshold;
        i32 m_numIterations;
        i32 m_solverMode;
        i32 m_restingContactRestitutionThreshold;
        i32 m_minimumSolverBatchSize;
        i32 m_splitImpulse;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class ContactSolverInfoFloatData
    {
    public:
        float m_tau;
        float m_damping;
        float m_friction;
        float m_timeStep;
        float m_restitution;
        float m_maxErrorReduction;
        float m_sor;
        float m_erp;
        float m_erp2;
        float m_globalCfm;
        float m_splitImpulsePenetrationThreshold;
        float m_splitImpulseTurnErp;
        float m_linearSlop;
        float m_warmstartingFactor;
        float m_articulatedWarmstartingFactor;
        float m_maxGyroscopicForce;
        float m_singleAxisRollingFrictionThreshold;
        i32 m_numIterations;
        i32 m_solverMode;
        i32 m_restingContactRestitutionThreshold;
        i32 m_minimumSolverBatchSize;
        i32 m_splitImpulse;
    };


// -------------------------------------------------- //
    class DynamicsWorldDoubleData
    {
    public:
        ContactSolverInfoDoubleData m_solverInfo;
        Vec3DoubleData m_gravity;
    };


// -------------------------------------------------- //
    class DynamicsWorldFloatData
    {
    public:
        ContactSolverInfoFloatData m_solverInfo;
        Vec3FloatData m_gravity;
    };


// -------------------------------------------------- //
    class RigidBodyFloatData
    {
    public:
        CollisionObject2FloatData m_collisionObjectData;
        Matrix3x3FloatData m_invInertiaTensorWorld;
        Vec3FloatData m_linearVelocity;
        Vec3FloatData m_angularVelocity;
        Vec3FloatData m_angularFactor;
        Vec3FloatData m_linearFactor;
        Vec3FloatData m_gravity;
        Vec3FloatData m_gravity_acceleration;
        Vec3FloatData m_invInertiaLocal;
        Vec3FloatData m_totalForce;
        Vec3FloatData m_totalTorque;
        float m_inverseMass;
        float m_linearDamping;
        float m_angularDamping;
        float m_additionalDampingFactor;
        float m_additionalLinearDampingThresholdSqr;
        float m_additionalAngularDampingThresholdSqr;
        float m_additionalAngularDampingFactor;
        float m_linearSleepingThreshold;
        float m_angularSleepingThreshold;
        i32 m_additionalDamping;
    };


// -------------------------------------------------- //
    class RigidBodyDoubleData
    {
    public:
        CollisionObject2DoubleData m_collisionObjectData;
        Matrix3x3DoubleData m_invInertiaTensorWorld;
        Vec3DoubleData m_linearVelocity;
        Vec3DoubleData m_angularVelocity;
        Vec3DoubleData m_angularFactor;
        Vec3DoubleData m_linearFactor;
        Vec3DoubleData m_gravity;
        Vec3DoubleData m_gravity_acceleration;
        Vec3DoubleData m_invInertiaLocal;
        Vec3DoubleData m_totalForce;
        Vec3DoubleData m_totalTorque;
        double m_inverseMass;
        double m_linearDamping;
        double m_angularDamping;
        double m_additionalDampingFactor;
        double m_additionalLinearDampingThresholdSqr;
        double m_additionalAngularDampingThresholdSqr;
        double m_additionalAngularDampingFactor;
        double m_linearSleepingThreshold;
        double m_angularSleepingThreshold;
        i32 m_additionalDamping;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class ConstraintInfo1
    {
    public:
        i32 m_numConstraintRows;
        i32 nub;
    };


// -------------------------------------------------- //
    class TypedConstraintFloatData
    {
    public:
        RigidBodyFloatData *m_rbA;
        RigidBodyFloatData *m_rbB;
        char *m_name;
        i32 m_objectType;
        i32 m_userConstraintType;
        i32 m_userConstraintId;
        i32 m_needsFeedback;
        float m_appliedImpulse;
        float m_dbgDrawSize;
        i32 m_disableCollisionsBetweenLinkedBodies;
        i32 m_overrideNumSolverIterations;
        float m_breakingImpulseThreshold;
        i32 m_isEnabled;
    };


// -------------------------------------------------- //
    class TypedConstraintData
    {
    public:
        bInvalidHandle *m_rbA;
        bInvalidHandle *m_rbB;
        char *m_name;
        i32 m_objectType;
        i32 m_userConstraintType;
        i32 m_userConstraintId;
        i32 m_needsFeedback;
        float m_appliedImpulse;
        float m_dbgDrawSize;
        i32 m_disableCollisionsBetweenLinkedBodies;
        i32 m_overrideNumSolverIterations;
        float m_breakingImpulseThreshold;
        i32 m_isEnabled;
    };


// -------------------------------------------------- //
    class TypedConstraintDoubleData
    {
    public:
        RigidBodyDoubleData *m_rbA;
        RigidBodyDoubleData *m_rbB;
        char *m_name;
        i32 m_objectType;
        i32 m_userConstraintType;
        i32 m_userConstraintId;
        i32 m_needsFeedback;
        double m_appliedImpulse;
        double m_dbgDrawSize;
        i32 m_disableCollisionsBetweenLinkedBodies;
        i32 m_overrideNumSolverIterations;
        double m_breakingImpulseThreshold;
        i32 m_isEnabled;
        char padding[4];
    };


// -------------------------------------------------- //
    class Point2PointConstraintFloatData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Vec3FloatData m_pivotInA;
        Vec3FloatData m_pivotInB;
    };


// -------------------------------------------------- //
    class Point2PointConstraintDoubleData2
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Vec3DoubleData m_pivotInA;
        Vec3DoubleData m_pivotInB;
    };


// -------------------------------------------------- //
    class Point2PointConstraintDoubleData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Vec3DoubleData m_pivotInA;
        Vec3DoubleData m_pivotInB;
    };


// -------------------------------------------------- //
    class HingeConstraintDoubleData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        i32 m_useReferenceFrameA;
        i32 m_angularOnly;
        i32 m_enableAngularMotor;
        float m_motorTargetVelocity;
        float m_maxMotorImpulse;
        float m_lowerLimit;
        float m_upperLimit;
        float m_limitSoftness;
        float m_biasFactor;
        float m_relaxationFactor;
    };


// -------------------------------------------------- //
    class HingeConstraintFloatData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2FloatData m_rbAFrame;
        Transform2FloatData m_rbBFrame;
        i32 m_useReferenceFrameA;
        i32 m_angularOnly;
        i32 m_enableAngularMotor;
        float m_motorTargetVelocity;
        float m_maxMotorImpulse;
        float m_lowerLimit;
        float m_upperLimit;
        float m_limitSoftness;
        float m_biasFactor;
        float m_relaxationFactor;
    };


// -------------------------------------------------- //
    class HingeConstraintDoubleData2
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        i32 m_useReferenceFrameA;
        i32 m_angularOnly;
        i32 m_enableAngularMotor;
        double m_motorTargetVelocity;
        double m_maxMotorImpulse;
        double m_lowerLimit;
        double m_upperLimit;
        double m_limitSoftness;
        double m_biasFactor;
        double m_relaxationFactor;
        char m_padding1[4];
    };


// -------------------------------------------------- //
    class ConeTwistConstraintDoubleData
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        double m_swingSpan1;
        double m_swingSpan2;
        double m_twistSpan;
        double m_limitSoftness;
        double m_biasFactor;
        double m_relaxationFactor;
        double m_damping;
    };


// -------------------------------------------------- //
    class ConeTwistConstraintData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2FloatData m_rbAFrame;
        Transform2FloatData m_rbBFrame;
        float m_swingSpan1;
        float m_swingSpan2;
        float m_twistSpan;
        float m_limitSoftness;
        float m_biasFactor;
        float m_relaxationFactor;
        float m_damping;
        char m_pad[4];
    };


// -------------------------------------------------- //
    class Generic6DofConstraintData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2FloatData m_rbAFrame;
        Transform2FloatData m_rbBFrame;
        Vec3FloatData m_linearUpperLimit;
        Vec3FloatData m_linearLowerLimit;
        Vec3FloatData m_angularUpperLimit;
        Vec3FloatData m_angularLowerLimit;
        i32 m_useLinearReferenceFrameA;
        i32 m_useOffsetForConstraintFrame;
    };


// -------------------------------------------------- //
    class Generic6DofConstraintDoubleData2
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        Vec3DoubleData m_linearUpperLimit;
        Vec3DoubleData m_linearLowerLimit;
        Vec3DoubleData m_angularUpperLimit;
        Vec3DoubleData m_angularLowerLimit;
        i32 m_useLinearReferenceFrameA;
        i32 m_useOffsetForConstraintFrame;
    };


// -------------------------------------------------- //
    class Generic6DofSpringConstraintData
    {
    public:
        Generic6DofConstraintData m_6dofData;
        i32 m_springEnabled[6];
        float m_equilibriumPoint[6];
        float m_springStiffness[6];
        float m_springDamping[6];
    };


// -------------------------------------------------- //
    class Generic6DofSpringConstraintDoubleData2
    {
    public:
        Generic6DofConstraintDoubleData2 m_6dofData;
        i32 m_springEnabled[6];
        double m_equilibriumPoint[6];
        double m_springStiffness[6];
        double m_springDamping[6];
    };


// -------------------------------------------------- //
    class Generic6DofSpring2ConstraintData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2FloatData m_rbAFrame;
        Transform2FloatData m_rbBFrame;
        Vec3FloatData m_linearUpperLimit;
        Vec3FloatData m_linearLowerLimit;
        Vec3FloatData m_linearBounce;
        Vec3FloatData m_linearStopERP;
        Vec3FloatData m_linearStopCFM;
        Vec3FloatData m_linearMotorERP;
        Vec3FloatData m_linearMotorCFM;
        Vec3FloatData m_linearTargetVelocity;
        Vec3FloatData m_linearMaxMotorForce;
        Vec3FloatData m_linearServoTarget;
        Vec3FloatData m_linearSpringStiffness;
        Vec3FloatData m_linearSpringDamping;
        Vec3FloatData m_linearEquilibriumPoint;
        char m_linearEnableMotor[4];
        char m_linearServoMotor[4];
        char m_linearEnableSpring[4];
        char m_linearSpringStiffnessLimited[4];
        char m_linearSpringDampingLimited[4];
        char m_padding1[4];
        Vec3FloatData m_angularUpperLimit;
        Vec3FloatData m_angularLowerLimit;
        Vec3FloatData m_angularBounce;
        Vec3FloatData m_angularStopERP;
        Vec3FloatData m_angularStopCFM;
        Vec3FloatData m_angularMotorERP;
        Vec3FloatData m_angularMotorCFM;
        Vec3FloatData m_angularTargetVelocity;
        Vec3FloatData m_angularMaxMotorForce;
        Vec3FloatData m_angularServoTarget;
        Vec3FloatData m_angularSpringStiffness;
        Vec3FloatData m_angularSpringDamping;
        Vec3FloatData m_angularEquilibriumPoint;
        char m_angularEnableMotor[4];
        char m_angularServoMotor[4];
        char m_angularEnableSpring[4];
        char m_angularSpringStiffnessLimited[4];
        char m_angularSpringDampingLimited[4];
        i32 m_rotateOrder;
    };


// -------------------------------------------------- //
    class Generic6DofSpring2ConstraintDoubleData2
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        Vec3DoubleData m_linearUpperLimit;
        Vec3DoubleData m_linearLowerLimit;
        Vec3DoubleData m_linearBounce;
        Vec3DoubleData m_linearStopERP;
        Vec3DoubleData m_linearStopCFM;
        Vec3DoubleData m_linearMotorERP;
        Vec3DoubleData m_linearMotorCFM;
        Vec3DoubleData m_linearTargetVelocity;
        Vec3DoubleData m_linearMaxMotorForce;
        Vec3DoubleData m_linearServoTarget;
        Vec3DoubleData m_linearSpringStiffness;
        Vec3DoubleData m_linearSpringDamping;
        Vec3DoubleData m_linearEquilibriumPoint;
        char m_linearEnableMotor[4];
        char m_linearServoMotor[4];
        char m_linearEnableSpring[4];
        char m_linearSpringStiffnessLimited[4];
        char m_linearSpringDampingLimited[4];
        char m_padding1[4];
        Vec3DoubleData m_angularUpperLimit;
        Vec3DoubleData m_angularLowerLimit;
        Vec3DoubleData m_angularBounce;
        Vec3DoubleData m_angularStopERP;
        Vec3DoubleData m_angularStopCFM;
        Vec3DoubleData m_angularMotorERP;
        Vec3DoubleData m_angularMotorCFM;
        Vec3DoubleData m_angularTargetVelocity;
        Vec3DoubleData m_angularMaxMotorForce;
        Vec3DoubleData m_angularServoTarget;
        Vec3DoubleData m_angularSpringStiffness;
        Vec3DoubleData m_angularSpringDamping;
        Vec3DoubleData m_angularEquilibriumPoint;
        char m_angularEnableMotor[4];
        char m_angularServoMotor[4];
        char m_angularEnableSpring[4];
        char m_angularSpringStiffnessLimited[4];
        char m_angularSpringDampingLimited[4];
        i32 m_rotateOrder;
    };


// -------------------------------------------------- //
    class SliderConstraintData
    {
    public:
        TypedConstraintData m_typeConstraintData;
        Transform2FloatData m_rbAFrame;
        Transform2FloatData m_rbBFrame;
        float m_linearUpperLimit;
        float m_linearLowerLimit;
        float m_angularUpperLimit;
        float m_angularLowerLimit;
        i32 m_useLinearReferenceFrameA;
        i32 m_useOffsetForConstraintFrame;
    };


// -------------------------------------------------- //
    class SliderConstraintDoubleData
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Transform2DoubleData m_rbAFrame;
        Transform2DoubleData m_rbBFrame;
        double m_linearUpperLimit;
        double m_linearLowerLimit;
        double m_angularUpperLimit;
        double m_angularLowerLimit;
        i32 m_useLinearReferenceFrameA;
        i32 m_useOffsetForConstraintFrame;
    };


// -------------------------------------------------- //
    class GearConstraintFloatData
    {
    public:
        TypedConstraintFloatData m_typeConstraintData;
        Vec3FloatData m_axisInA;
        Vec3FloatData m_axisInB;
        float m_ratio;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class GearConstraintDoubleData
    {
    public:
        TypedConstraintDoubleData m_typeConstraintData;
        Vec3DoubleData m_axisInA;
        Vec3DoubleData m_axisInB;
        double m_ratio;
    };


// -------------------------------------------------- //
    class SoftBodyMaterialData
    {
    public:
        float m_linearStiffness;
        float m_angularStiffness;
        float m_volumeStiffness;
        i32 m_flags;
    };


// -------------------------------------------------- //
    class SoftBodyNodeData
    {
    public:
        SoftBodyMaterialData *m_material;
        Vec3FloatData m_position;
        Vec3FloatData m_previousPosition;
        Vec3FloatData m_velocity;
        Vec3FloatData m_accumulatedForce;
        Vec3FloatData m_normal;
        float m_inverseMass;
        float m_area;
        i32 m_attach;
        i32 m_pad;
    };


// -------------------------------------------------- //
    class SoftBodyLinkData
    {
    public:
        SoftBodyMaterialData *m_material;
        i32 m_nodeIndices[2];
        float m_restLength;
        i32 m_bbending;
    };


// -------------------------------------------------- //
    class SoftBodyFaceData
    {
    public:
        Vec3FloatData m_normal;
        SoftBodyMaterialData *m_material;
        i32 m_nodeIndices[3];
        float m_restArea;
    };


// -------------------------------------------------- //
    class SoftBodyTetraData
    {
    public:
        Vec3FloatData m_c0[4];
        SoftBodyMaterialData *m_material;
        i32 m_nodeIndices[4];
        float m_restVolume;
        float m_c1;
        float m_c2;
        i32 m_pad;
    };


// -------------------------------------------------- //
    class SoftRigidAnchorData
    {
    public:
        Matrix3x3FloatData m_c0;
        Vec3FloatData m_c1;
        Vec3FloatData m_localFrame;
        bInvalidHandle *m_rigidBody;
        i32 m_nodeIndex;
        float m_c2;
    };


// -------------------------------------------------- //
    class SoftBodyConfigData
    {
    public:
        i32 m_aeroModel;
        float m_baumgarte;
        float m_damping;
        float m_drag;
        float m_lift;
        float m_pressure;
        float m_volume;
        float m_dynamicFriction;
        float m_poseMatch;
        float m_rigidContactHardness;
        float m_kineticContactHardness;
        float m_softContactHardness;
        float m_anchorHardness;
        float m_softRigidClusterHardness;
        float m_softKineticClusterHardness;
        float m_softSoftClusterHardness;
        float m_softRigidClusterImpulseSplit;
        float m_softKineticClusterImpulseSplit;
        float m_softSoftClusterImpulseSplit;
        float m_maxVolume;
        float m_timeScale;
        i32 m_velocityIterations;
        i32 m_positionIterations;
        i32 m_driftIterations;
        i32 m_clusterIterations;
        i32 m_collisionFlags;
    };


// -------------------------------------------------- //
    class SoftBodyPoseData
    {
    public:
        Matrix3x3FloatData m_rot;
        Matrix3x3FloatData m_scale;
        Matrix3x3FloatData m_aqq;
        Vec3FloatData m_com;
        Vec3FloatData *m_positions;
        float *m_weights;
        i32 m_numPositions;
        i32 m_numWeigts;
        i32 m_bvolume;
        i32 m_bframe;
        float m_restVolume;
        i32 m_pad;
    };


// -------------------------------------------------- //
    class SoftBodyClusterData
    {
    public:
        Transform2FloatData m_framexform;
        Matrix3x3FloatData m_locii;
        Matrix3x3FloatData m_invwi;
        Vec3FloatData m_com;
        Vec3FloatData m_vimpulses[2];
        Vec3FloatData m_dimpulses[2];
        Vec3FloatData m_lv;
        Vec3FloatData m_av;
        Vec3FloatData *m_framerefs;
        i32 *m_nodeIndices;
        float *m_masses;
        i32 m_numFrameRefs;
        i32 m_numNodes;
        i32 m_numMasses;
        float m_idmass;
        float m_imass;
        i32 m_nvimpulses;
        i32 m_ndimpulses;
        float m_ndamping;
        float m_ldamping;
        float m_adamping;
        float m_matching;
        float m_maxSelfCollisionImpulse;
        float m_selfCollisionImpulseFactor;
        i32 m_containsAnchor;
        i32 m_collide;
        i32 m_clusterIndex;
    };


// -------------------------------------------------- //
    class SoftBodyJointData
    {
    public:
        uk m_bodyA;
        uk m_bodyB;
        Vec3FloatData m_refs[2];
        float m_cfm;
        float m_erp;
        float m_split;
        i32 m_delete;
        Vec3FloatData m_relPosition[2];
        i32 m_bodyAtype;
        i32 m_bodyBtype;
        i32 m_jointType;
        i32 m_pad;
    };


// -------------------------------------------------- //
    class SoftBodyFloatData
    {
    public:
        CollisionObject2FloatData m_collisionObjectData;
        SoftBodyPoseData *m_pose;
        SoftBodyMaterialData **m_materials;
        SoftBodyNodeData *m_nodes;
        SoftBodyLinkData *m_links;
        SoftBodyFaceData *m_faces;
        SoftBodyTetraData *m_tetrahedra;
        SoftRigidAnchorData *m_anchors;
        SoftBodyClusterData *m_clusters;
        SoftBodyJointData *m_joints;
        i32 m_numMaterials;
        i32 m_numNodes;
        i32 m_numLinks;
        i32 m_numFaces;
        i32 m_numTetrahedra;
        i32 m_numAnchors;
        i32 m_numClusters;
        i32 m_numJoints;
        SoftBodyConfigData m_config;
    };


// -------------------------------------------------- //
    class MultiBodyLinkDoubleData
    {
    public:
        QuatDoubleData m_zeroRotParentToThis;
        Vec3DoubleData m_parentComToThisPivotOffset;
        Vec3DoubleData m_thisPivotToThisComOffset;
        Vec3DoubleData m_jointAxisTop[6];
        Vec3DoubleData m_jointAxisBottom[6];
        Vec3DoubleData m_linkInertia;
        Vec3DoubleData m_absFrameTotVelocityTop;
        Vec3DoubleData m_absFrameTotVelocityBottom;
        Vec3DoubleData m_absFrameLocVelocityTop;
        Vec3DoubleData m_absFrameLocVelocityBottom;
        double m_linkMass;
        i32 m_parentIndex;
        i32 m_jointType;
        i32 m_dofCount;
        i32 m_posVarCount;
        double m_jointPos[7];
        double m_jointVel[6];
        double m_jointTorque[6];
        double m_jointDamping;
        double m_jointFriction;
        double m_jointLowerLimit;
        double m_jointUpperLimit;
        double m_jointMaxForce;
        double m_jointMaxVelocity;
        char *m_linkName;
        char *m_jointName;
        CollisionObject2DoubleData *m_linkCollider;
        char *m_paddingPtr;
    };


// -------------------------------------------------- //
    class MultiBodyLinkFloatData
    {
    public:
        QuatFloatData m_zeroRotParentToThis;
        Vec3FloatData m_parentComToThisPivotOffset;
        Vec3FloatData m_thisPivotToThisComOffset;
        Vec3FloatData m_jointAxisTop[6];
        Vec3FloatData m_jointAxisBottom[6];
        Vec3FloatData m_linkInertia;
        Vec3FloatData m_absFrameTotVelocityTop;
        Vec3FloatData m_absFrameTotVelocityBottom;
        Vec3FloatData m_absFrameLocVelocityTop;
        Vec3FloatData m_absFrameLocVelocityBottom;
        i32 m_dofCount;
        float m_linkMass;
        i32 m_parentIndex;
        i32 m_jointType;
        float m_jointPos[7];
        float m_jointVel[6];
        float m_jointTorque[6];
        i32 m_posVarCount;
        float m_jointDamping;
        float m_jointFriction;
        float m_jointLowerLimit;
        float m_jointUpperLimit;
        float m_jointMaxForce;
        float m_jointMaxVelocity;
        char *m_linkName;
        char *m_jointName;
        CollisionObject2FloatData *m_linkCollider;
        char *m_paddingPtr;
    };


// -------------------------------------------------- //
    class MultiBodyDoubleData
    {
    public:
        Vec3DoubleData m_baseWorldPosition;
        QuatDoubleData m_baseWorldOrientation;
        Vec3DoubleData m_baseLinearVelocity;
        Vec3DoubleData m_baseAngularVelocity;
        Vec3DoubleData m_baseInertia;
        double m_baseMass;
        i32 m_numLinks;
        char m_padding[4];
        char *m_baseName;
        MultiBodyLinkDoubleData *m_links;
        CollisionObject2DoubleData *m_baseCollider;
    };


// -------------------------------------------------- //
    class MultiBodyFloatData
    {
    public:
        Vec3FloatData m_baseWorldPosition;
        QuatFloatData m_baseWorldOrientation;
        Vec3FloatData m_baseLinearVelocity;
        Vec3FloatData m_baseAngularVelocity;
        Vec3FloatData m_baseInertia;
        float m_baseMass;
        i32 m_numLinks;
        char *m_baseName;
        MultiBodyLinkFloatData *m_links;
        CollisionObject2FloatData *m_baseCollider;
    };


// -------------------------------------------------- //
    class MultiBodyLinkColliderFloatData
    {
    public:
        CollisionObject2FloatData m_colObjData;
        MultiBodyFloatData *m_multiBody;
        i32 m_link;
        char m_padding[4];
    };


// -------------------------------------------------- //
    class MultiBodyLinkColliderDoubleData
    {
    public:
        CollisionObject2DoubleData m_colObjData;
        MultiBodyDoubleData *m_multiBody;
        i32 m_link;
        char m_padding[4];
    };


}
#endif//__DRX3D_H__