// Auto generated from drx3D/Extras/HeaderGenerator/bulletGenerate.py
#ifndef __BULLET2_H__
#define __BULLET2_H__
namespace Bullet3SerializeBullet2
{
// put an empty struct in the case
typedef struct bInvalidHandle
{
	i32 unused;
} bInvalidHandle;

class PointerArray;
class b3PhysicsSystem;
class ListBase;
class b3Vec3FloatData;
class b3Vec3DoubleData;
class b3Matrix3x3FloatData;
class b3Matrix3x3DoubleData;
class b3TransformFloatData;
class b3TransformDoubleData;
class b3BvhSubtreeInfoData;
class b3OptimizedBvhNodeFloatData;
class b3OptimizedBvhNodeDoubleData;
class b3QuantizedBvhNodeData;
class b3QuantizedBvhFloatData;
class b3QuantizedBvhDoubleData;
class b3CollisionShapeData;
class b3StaticPlaneShapeData;
class b3ConvexInternalShapeData;
class b3PositionAndRadius;
class b3MultiSphereShapeData;
class b3IntIndexData;
class b3ShortIntIndexData;
class b3ShortIntIndexTripletData;
class b3CharIndexTripletData;
class b3MeshPartData;
class b3StridingMeshInterfaceData;
class b3TriangleMeshShapeData;
class b3ScaledTriangleMeshShapeData;
class b3CompoundShapeChildData;
class b3CompoundShapeData;
class b3CylinderShapeData;
class b3CapsuleShapeData;
class b3TriangleInfoData;
class b3TriangleInfoMapData;
class b3GImpactMeshShapeData;
class b3ConvexHullShapeData;
class b3CollisionObjectDoubleData;
class b3CollisionObjectFloatData;
class b3DynamicsWorldDoubleData;
class b3DynamicsWorldFloatData;
class b3RigidBodyFloatData;
class b3RigidBodyDoubleData;
class b3ConstraintInfo1;
class b3TypedConstraintData;
class b3Point2PointConstraintFloatData;
class b3Point2PointConstraintDoubleData;
class b3HingeConstraintDoubleData;
class b3HingeConstraintFloatData;
class b3ConeTwistConstraintData;
class b3Generic6DofConstraintData;
class b3Generic6DofSpringConstraintData;
class b3SliderConstraintData;
class b3ContactSolverInfoDoubleData;
class b3ContactSolverInfoFloatData;
class SoftBodyMaterialData;
class SoftBodyNodeData;
class SoftBodyLinkData;
class SoftBodyFaceData;
class SoftBodyTetraData;
class SoftRigidAnchorData;
class SoftBodyConfigData;
class SoftBodyPoseData;
class SoftBodyClusterData;
class b3SoftBodyJointData;
class b3SoftBodyFloatData;
// -------------------------------------------------- //
class PointerArray
{
public:
	i32 m_size;
	i32 m_capacity;
	uk m_data;
};

// -------------------------------------------------- //
class b3PhysicsSystem
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
class b3Vec3FloatData
{
public:
	float m_floats[4];
};

// -------------------------------------------------- //
class b3Vec3DoubleData
{
public:
	double m_floats[4];
};

// -------------------------------------------------- //
class b3Matrix3x3FloatData
{
public:
	b3Vec3FloatData m_el[3];
};

// -------------------------------------------------- //
class b3Matrix3x3DoubleData
{
public:
	b3Vec3DoubleData m_el[3];
};

// -------------------------------------------------- //
class b3TransformFloatData
{
public:
	b3Matrix3x3FloatData m_basis;
	b3Vec3FloatData m_origin;
};

// -------------------------------------------------- //
class b3TransformDoubleData
{
public:
	b3Matrix3x3DoubleData m_basis;
	b3Vec3DoubleData m_origin;
};

// -------------------------------------------------- //
class b3BvhSubtreeInfoData
{
public:
	i32 m_rootNodeIndex;
	i32 m_subtreeSize;
	short m_quantizedAabbMin[3];
	short m_quantizedAabbMax[3];
};

// -------------------------------------------------- //
class b3OptimizedBvhNodeFloatData
{
public:
	b3Vec3FloatData m_aabbMinOrg;
	b3Vec3FloatData m_aabbMaxOrg;
	i32 m_escapeIndex;
	i32 m_subPart;
	i32 m_triangleIndex;
	char m_pad[4];
};

// -------------------------------------------------- //
class b3OptimizedBvhNodeDoubleData
{
public:
	b3Vec3DoubleData m_aabbMinOrg;
	b3Vec3DoubleData m_aabbMaxOrg;
	i32 m_escapeIndex;
	i32 m_subPart;
	i32 m_triangleIndex;
	char m_pad[4];
};

// -------------------------------------------------- //
class b3QuantizedBvhNodeData
{
public:
	short m_quantizedAabbMin[3];
	short m_quantizedAabbMax[3];
	i32 m_escapeIndexOrTriangleIndex;
};

// -------------------------------------------------- //
class b3QuantizedBvhFloatData
{
public:
	b3Vec3FloatData m_bvhAabbMin;
	b3Vec3FloatData m_bvhAabbMax;
	b3Vec3FloatData m_bvhQuantization;
	i32 m_curNodeIndex;
	i32 m_useQuantization;
	i32 m_numContiguousLeafNodes;
	i32 m_numQuantizedContiguousNodes;
	b3OptimizedBvhNodeFloatData *m_contiguousNodesPtr;
	b3QuantizedBvhNodeData *m_quantizedContiguousNodesPtr;
	b3BvhSubtreeInfoData *m_subTreeInfoPtr;
	i32 m_traversalMode;
	i32 m_numSubtreeHeaders;
};

// -------------------------------------------------- //
class b3QuantizedBvhDoubleData
{
public:
	b3Vec3DoubleData m_bvhAabbMin;
	b3Vec3DoubleData m_bvhAabbMax;
	b3Vec3DoubleData m_bvhQuantization;
	i32 m_curNodeIndex;
	i32 m_useQuantization;
	i32 m_numContiguousLeafNodes;
	i32 m_numQuantizedContiguousNodes;
	b3OptimizedBvhNodeDoubleData *m_contiguousNodesPtr;
	b3QuantizedBvhNodeData *m_quantizedContiguousNodesPtr;
	i32 m_traversalMode;
	i32 m_numSubtreeHeaders;
	b3BvhSubtreeInfoData *m_subTreeInfoPtr;
};

// -------------------------------------------------- //
class b3CollisionShapeData
{
public:
	char *m_name;
	i32 m_shapeType;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3StaticPlaneShapeData
{
public:
	b3CollisionShapeData m_collisionShapeData;
	b3Vec3FloatData m_localScaling;
	b3Vec3FloatData m_planeNormal;
	float m_planeConstant;
	char m_pad[4];
};

// -------------------------------------------------- //
class b3ConvexInternalShapeData
{
public:
	b3CollisionShapeData m_collisionShapeData;
	b3Vec3FloatData m_localScaling;
	b3Vec3FloatData m_implicitShapeDimensions;
	float m_collisionMargin;
	i32 m_padding;
};

// -------------------------------------------------- //
class b3PositionAndRadius
{
public:
	b3Vec3FloatData m_pos;
	float m_radius;
};

// -------------------------------------------------- //
class b3MultiSphereShapeData
{
public:
	b3ConvexInternalShapeData m_convexInternalShapeData;
	b3PositionAndRadius *m_localPositionArrayPtr;
	i32 m_localPositionArraySize;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3IntIndexData
{
public:
	i32 m_value;
};

// -------------------------------------------------- //
class b3ShortIntIndexData
{
public:
	short m_value;
	char m_pad[2];
};

// -------------------------------------------------- //
class b3ShortIntIndexTripletData
{
public:
	short m_values[3];
	char m_pad[2];
};

// -------------------------------------------------- //
class b3CharIndexTripletData
{
public:
	char m_values[3];
	char m_pad;
};

// -------------------------------------------------- //
class b3MeshPartData
{
public:
	b3Vec3FloatData *m_vertices3f;
	b3Vec3DoubleData *m_vertices3d;
	b3IntIndexData *m_indices32;
	b3ShortIntIndexTripletData *m_3indices16;
	b3CharIndexTripletData *m_3indices8;
	b3ShortIntIndexData *m_indices16;
	i32 m_numTriangles;
	i32 m_numVertices;
};

// -------------------------------------------------- //
class b3StridingMeshInterfaceData
{
public:
	b3MeshPartData *m_meshPartsPtr;
	b3Vec3FloatData m_scaling;
	i32 m_numMeshParts;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3TriangleMeshShapeData
{
public:
	b3CollisionShapeData m_collisionShapeData;
	b3StridingMeshInterfaceData m_meshInterface;
	b3QuantizedBvhFloatData *m_quantizedFloatBvh;
	b3QuantizedBvhDoubleData *m_quantizedDoubleBvh;
	b3TriangleInfoMapData *m_triangleInfoMap;
	float m_collisionMargin;
	char m_pad3[4];
};

// -------------------------------------------------- //
class b3ScaledTriangleMeshShapeData
{
public:
	b3TriangleMeshShapeData m_trimeshShapeData;
	b3Vec3FloatData m_localScaling;
};

// -------------------------------------------------- //
class b3CompoundShapeChildData
{
public:
	b3TransformFloatData m_transform;
	b3CollisionShapeData *m_childShape;
	i32 m_childShapeType;
	float m_childMargin;
};

// -------------------------------------------------- //
class b3CompoundShapeData
{
public:
	b3CollisionShapeData m_collisionShapeData;
	b3CompoundShapeChildData *m_childShapePtr;
	i32 m_numChildShapes;
	float m_collisionMargin;
};

// -------------------------------------------------- //
class b3CylinderShapeData
{
public:
	b3ConvexInternalShapeData m_convexInternalShapeData;
	i32 m_upAxis;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3CapsuleShapeData
{
public:
	b3ConvexInternalShapeData m_convexInternalShapeData;
	i32 m_upAxis;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3TriangleInfoData
{
public:
	i32 m_flags;
	float m_edgeV0V1Angle;
	float m_edgeV1V2Angle;
	float m_edgeV2V0Angle;
};

// -------------------------------------------------- //
class b3TriangleInfoMapData
{
public:
	i32 *m_hashTablePtr;
	i32 *m_nextPtr;
	b3TriangleInfoData *m_valueArrayPtr;
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
class b3GImpactMeshShapeData
{
public:
	b3CollisionShapeData m_collisionShapeData;
	b3StridingMeshInterfaceData m_meshInterface;
	b3Vec3FloatData m_localScaling;
	float m_collisionMargin;
	i32 m_gimpactSubType;
};

// -------------------------------------------------- //
class b3ConvexHullShapeData
{
public:
	b3ConvexInternalShapeData m_convexInternalShapeData;
	b3Vec3FloatData *m_unscaledPointsFloatPtr;
	b3Vec3DoubleData *m_unscaledPointsDoublePtr;
	i32 m_numUnscaledPoints;
	char m_padding3[4];
};

// -------------------------------------------------- //
class b3CollisionObjectDoubleData
{
public:
	uk m_broadphaseHandle;
	uk m_collisionShape;
	b3CollisionShapeData *m_rootCollisionShape;
	char *m_name;
	b3TransformDoubleData m_worldTransform;
	b3TransformDoubleData m_interpolationWorldTransform;
	b3Vec3DoubleData m_interpolationLinearVelocity;
	b3Vec3DoubleData m_interpolationAngularVelocity;
	b3Vec3DoubleData m_anisotropicFriction;
	double m_contactProcessingThreshold;
	double m_deactivationTime;
	double m_friction;
	double m_rollingFriction;
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
	char m_padding[4];
};

// -------------------------------------------------- //
class b3CollisionObjectFloatData
{
public:
	uk m_broadphaseHandle;
	uk m_collisionShape;
	b3CollisionShapeData *m_rootCollisionShape;
	char *m_name;
	b3TransformFloatData m_worldTransform;
	b3TransformFloatData m_interpolationWorldTransform;
	b3Vec3FloatData m_interpolationLinearVelocity;
	b3Vec3FloatData m_interpolationAngularVelocity;
	b3Vec3FloatData m_anisotropicFriction;
	float m_contactProcessingThreshold;
	float m_deactivationTime;
	float m_friction;
	float m_rollingFriction;
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
	char m_padding[4];
};

// -------------------------------------------------- //
class b3RigidBodyFloatData
{
public:
	b3CollisionObjectFloatData m_collisionObjectData;
	b3Matrix3x3FloatData m_invInertiaTensorWorld;
	b3Vec3FloatData m_linearVelocity;
	b3Vec3FloatData m_angularVelocity;
	b3Vec3FloatData m_angularFactor;
	b3Vec3FloatData m_linearFactor;
	b3Vec3FloatData m_gravity;
	b3Vec3FloatData m_gravity_acceleration;
	b3Vec3FloatData m_invInertiaLocal;
	b3Vec3FloatData m_totalForce;
	b3Vec3FloatData m_totalTorque;
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
class b3RigidBodyDoubleData
{
public:
	b3CollisionObjectDoubleData m_collisionObjectData;
	b3Matrix3x3DoubleData m_invInertiaTensorWorld;
	b3Vec3DoubleData m_linearVelocity;
	b3Vec3DoubleData m_angularVelocity;
	b3Vec3DoubleData m_angularFactor;
	b3Vec3DoubleData m_linearFactor;
	b3Vec3DoubleData m_gravity;
	b3Vec3DoubleData m_gravity_acceleration;
	b3Vec3DoubleData m_invInertiaLocal;
	b3Vec3DoubleData m_totalForce;
	b3Vec3DoubleData m_totalTorque;
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
class b3ConstraintInfo1
{
public:
	i32 m_numConstraintRows;
	i32 nub;
};

// -------------------------------------------------- //
class b3TypedConstraintData
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
class b3Point2PointConstraintFloatData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3Vec3FloatData m_pivotInA;
	b3Vec3FloatData m_pivotInB;
};

// -------------------------------------------------- //
class b3Point2PointConstraintDoubleData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3Vec3DoubleData m_pivotInA;
	b3Vec3DoubleData m_pivotInB;
};

// -------------------------------------------------- //
class b3HingeConstraintDoubleData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3TransformDoubleData m_rbAFrame;
	b3TransformDoubleData m_rbBFrame;
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
class b3HingeConstraintFloatData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3TransformFloatData m_rbAFrame;
	b3TransformFloatData m_rbBFrame;
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
class b3ConeTwistConstraintData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3TransformFloatData m_rbAFrame;
	b3TransformFloatData m_rbBFrame;
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
class b3Generic6DofConstraintData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3TransformFloatData m_rbAFrame;
	b3TransformFloatData m_rbBFrame;
	b3Vec3FloatData m_linearUpperLimit;
	b3Vec3FloatData m_linearLowerLimit;
	b3Vec3FloatData m_angularUpperLimit;
	b3Vec3FloatData m_angularLowerLimit;
	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

// -------------------------------------------------- //
class b3Generic6DofSpringConstraintData
{
public:
	b3Generic6DofConstraintData m_6dofData;
	i32 m_springEnabled[6];
	float m_equilibriumPoint[6];
	float m_springStiffness[6];
	float m_springDamping[6];
};

// -------------------------------------------------- //
class b3SliderConstraintData
{
public:
	b3TypedConstraintData m_typeConstraintData;
	b3TransformFloatData m_rbAFrame;
	b3TransformFloatData m_rbBFrame;
	float m_linearUpperLimit;
	float m_linearLowerLimit;
	float m_angularUpperLimit;
	float m_angularLowerLimit;
	i32 m_useLinearReferenceFrameA;
	i32 m_useOffsetForConstraintFrame;
};

// -------------------------------------------------- //
class b3ContactSolverInfoDoubleData
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
class b3ContactSolverInfoFloatData
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
	float m_maxGyroscopicForce;
	float m_singleAxisRollingFrictionThreshold;
	i32 m_numIterations;
	i32 m_solverMode;
	i32 m_restingContactRestitutionThreshold;
	i32 m_minimumSolverBatchSize;
	i32 m_splitImpulse;
	char m_padding[4];
};

// -------------------------------------------------- //
class b3DynamicsWorldDoubleData
{
public:
	b3ContactSolverInfoDoubleData m_solverInfo;
	b3Vec3DoubleData m_gravity;
};

// -------------------------------------------------- //
class b3DynamicsWorldFloatData
{
public:
	b3ContactSolverInfoFloatData m_solverInfo;
	b3Vec3FloatData m_gravity;
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
	b3Vec3FloatData m_position;
	b3Vec3FloatData m_previousPosition;
	b3Vec3FloatData m_velocity;
	b3Vec3FloatData m_accumulatedForce;
	b3Vec3FloatData m_normal;
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
	b3Vec3FloatData m_normal;
	SoftBodyMaterialData *m_material;
	i32 m_nodeIndices[3];
	float m_restArea;
};

// -------------------------------------------------- //
class SoftBodyTetraData
{
public:
	b3Vec3FloatData m_c0[4];
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
	b3Matrix3x3FloatData m_c0;
	b3Vec3FloatData m_c1;
	b3Vec3FloatData m_localFrame;
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
	b3Matrix3x3FloatData m_rot;
	b3Matrix3x3FloatData m_scale;
	b3Matrix3x3FloatData m_aqq;
	b3Vec3FloatData m_com;
	b3Vec3FloatData *m_positions;
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
	b3TransformFloatData m_framexform;
	b3Matrix3x3FloatData m_locii;
	b3Matrix3x3FloatData m_invwi;
	b3Vec3FloatData m_com;
	b3Vec3FloatData m_vimpulses[2];
	b3Vec3FloatData m_dimpulses[2];
	b3Vec3FloatData m_lv;
	b3Vec3FloatData m_av;
	b3Vec3FloatData *m_framerefs;
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
class b3SoftBodyJointData
{
public:
	uk m_bodyA;
	uk m_bodyB;
	b3Vec3FloatData m_refs[2];
	float m_cfm;
	float m_erp;
	float m_split;
	i32 m_delete;
	b3Vec3FloatData m_relPosition[2];
	i32 m_bodyAtype;
	i32 m_bodyBtype;
	i32 m_jointType;
	i32 m_pad;
};

// -------------------------------------------------- //
class b3SoftBodyFloatData
{
public:
	b3CollisionObjectFloatData m_collisionObjectData;
	SoftBodyPoseData *m_pose;
	SoftBodyMaterialData **m_materials;
	SoftBodyNodeData *m_nodes;
	SoftBodyLinkData *m_links;
	SoftBodyFaceData *m_faces;
	SoftBodyTetraData *m_tetrahedra;
	SoftRigidAnchorData *m_anchors;
	SoftBodyClusterData *m_clusters;
	b3SoftBodyJointData *m_joints;
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

}  // namespace Bullet3SerializeBullet2
#endif  //__BULLET2_H__