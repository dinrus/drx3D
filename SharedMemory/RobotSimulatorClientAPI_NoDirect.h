#ifndef D3_ROBOT_SIMULATOR_CLIENT_API_NO_DIRECT_H
#define D3_ROBOT_SIMULATOR_CLIENT_API_NO_DIRECT_H

///The RobotSimulatorClientAPI is pretty much the C++ version of pybullet
///as documented in the pybullet Quickstart Guide
///https://docs.google.com/document/d/10sXEhzFRSnvFcl3XxNGhnD4N2SedqwdAvK3dsihxVUA

#include <drx3D/SharedMemory/SharedMemoryPublic.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/Quat.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/AlignedObjectArray.h>

#include <string>

struct RobotSimulatorLoadUrdfFileArgs
{
	Vec3 m_startPosition;
	Quat m_startOrientation;
	bool m_forceOverrideFixedBase;
	bool m_useMultiBody;
	i32 m_flags;

	RobotSimulatorLoadUrdfFileArgs(const Vec3 &startPos, const Quat &startOrn)
		: m_startPosition(startPos),
		  m_startOrientation(startOrn),
		  m_forceOverrideFixedBase(false),
		  m_useMultiBody(true),
		  m_flags(0)
	{
	}

	RobotSimulatorLoadUrdfFileArgs()
		: m_startPosition(Vec3(0, 0, 0)),
		  m_startOrientation(Quat(0, 0, 0, 1)),
		  m_forceOverrideFixedBase(false),
		  m_useMultiBody(true),
		  m_flags(0)
	{
	}
};

struct RobotSimulatorLoadSdfFileArgs
{
	bool m_forceOverrideFixedBase;
	bool m_useMultiBody;

	RobotSimulatorLoadSdfFileArgs()
		: m_forceOverrideFixedBase(false),
		  m_useMultiBody(true)
	{
	}
};

struct RobotSimulatorLoadSoftBodyArgs
{
	Vec3 m_startPosition;
	Quat m_startOrientation;
	double m_scale;
	double m_mass;
	double m_collisionMargin;

	RobotSimulatorLoadSoftBodyArgs(const Vec3 &startPos, const Quat &startOrn, const double &scale, const double &mass, const double &collisionMargin)
		: m_startPosition(startPos),
		  m_startOrientation(startOrn),
		  m_scale(scale),
		  m_mass(mass),
		  m_collisionMargin(collisionMargin)
	{
	}

	RobotSimulatorLoadSoftBodyArgs(const Vec3 &startPos, const Quat &startOrn)
	{
		RobotSimulatorLoadSoftBodyArgs(startPos, startOrn, 1.0, 1.0, 0.02);
	}

	RobotSimulatorLoadSoftBodyArgs()
	{
		RobotSimulatorLoadSoftBodyArgs(Vec3(0, 0, 0), Quat(0, 0, 0, 1));
	}

	RobotSimulatorLoadSoftBodyArgs(double scale, double mass, double collisionMargin)
		: m_startPosition(Vec3(0, 0, 0)),
		  m_startOrientation(Quat(0, 0, 0, 1)),
		  m_scale(scale),
		  m_mass(mass),
		  m_collisionMargin(collisionMargin)
	{
	}
};


struct RobotSimulatorLoadDeformableBodyArgs
{
	Vec3 m_startPosition;
	Quat m_startOrientation;
	double m_scale;
	double m_mass;
	double m_collisionMargin;
	double m_springElasticStiffness;
	double m_springDampingStiffness;
	double m_springBendingStiffness;
	double m_NeoHookeanMu;
	double m_NeoHookeanLambda;
	double m_NeoHookeanDamping;
	bool m_useSelfCollision;
	bool m_useFaceContact;
	bool m_useBendingSprings;
	double m_frictionCoeff;

	RobotSimulatorLoadDeformableBodyArgs(const Vec3 &startPos, const Quat &startOrn, const double &scale, const double &mass, const double &collisionMargin)
	: m_startPosition(startPos),
	m_startOrientation(startOrn),
	m_scale(scale),
	m_mass(mass),
	m_collisionMargin(collisionMargin),
	m_springElasticStiffness(-1),
	m_springDampingStiffness(-1),
	m_springBendingStiffness(-1),
	m_NeoHookeanMu(-1),
	m_NeoHookeanDamping(-1),
	m_useSelfCollision(false),
	m_useFaceContact(false),
	m_useBendingSprings(false),
	m_frictionCoeff(0)
	{
	}

	RobotSimulatorLoadDeformableBodyArgs(const Vec3 &startPos, const Quat &startOrn)
	{
		RobotSimulatorLoadSoftBodyArgs(startPos, startOrn, 1.0, 1.0, 0.02);
	}

	RobotSimulatorLoadDeformableBodyArgs()
	{
		RobotSimulatorLoadSoftBodyArgs(Vec3(0, 0, 0), Quat(0, 0, 0, 1));
	}

	RobotSimulatorLoadDeformableBodyArgs(double scale, double mass, double collisionMargin)
	: m_startPosition(Vec3(0, 0, 0)),
	m_startOrientation(Quat(0, 0, 0, 1)),
	m_scale(scale),
	m_mass(mass),
	m_collisionMargin(collisionMargin)
	{
	}
};


struct RobotSimulatorLoadFileResults
{
	AlignedObjectArray<i32> m_uniqueObjectIds;
	RobotSimulatorLoadFileResults()
	{
	}
};

struct RobotSimulatorChangeVisualShapeArgs
{
	i32 m_objectUniqueId;
	i32 m_linkIndex;
	i32 m_shapeIndex;
	i32 m_textureUniqueId;
	Vec4 m_rgbaColor;
	bool m_hasRgbaColor;
	Vec3 m_specularColor;
	bool m_hasSpecularColor;

	RobotSimulatorChangeVisualShapeArgs()
		: m_objectUniqueId(-1),
		  m_linkIndex(-1),
		  m_shapeIndex(-1),
		  m_textureUniqueId(-2),
		  m_rgbaColor(0, 0, 0, 1),
		  m_hasRgbaColor(false),
		  m_specularColor(1, 1, 1),
		  m_hasSpecularColor(false)
	{
	}
};

struct RobotSimulatorJointMotorArgs
{
	i32 m_controlMode;

	double m_targetPosition;
	double m_kp;

	double m_targetVelocity;
	double m_kd;

	double m_maxTorqueValue;

	RobotSimulatorJointMotorArgs(i32 controlMode)
		: m_controlMode(controlMode),
		  m_targetPosition(0),
		  m_kp(0.1),
		  m_targetVelocity(0),
		  m_kd(0.9),
		  m_maxTorqueValue(1000)
	{
	}
};

enum RobotSimulatorInverseKinematicsFlags
{
	D3_HAS_IK_TARGET_ORIENTATION = 1,
	D3_HAS_NULL_SPACE_VELOCITY = 2,
	D3_HAS_JOINT_DAMPING = 4,
	D3_HAS_CURRENT_POSITIONS = 8,
};

struct RobotSimulatorInverseKinematicArgs
{
	i32 m_bodyUniqueId;
	double m_endEffectorTargetPosition[3];
	double m_endEffectorTargetOrientation[4];
	i32 m_endEffectorLinkIndex;
	i32 m_flags;
	i32 m_numDegreeOfFreedom;
	AlignedObjectArray<double> m_lowerLimits;
	AlignedObjectArray<double> m_upperLimits;
	AlignedObjectArray<double> m_jointRanges;
	AlignedObjectArray<double> m_restPoses;
	AlignedObjectArray<double> m_jointDamping;
	AlignedObjectArray<double> m_currentJointPositions;

	RobotSimulatorInverseKinematicArgs()
		: m_bodyUniqueId(-1),
		  m_endEffectorLinkIndex(-1),
		  m_flags(0)
	{
		m_endEffectorTargetPosition[0] = 0;
		m_endEffectorTargetPosition[1] = 0;
		m_endEffectorTargetPosition[2] = 0;

		m_endEffectorTargetOrientation[0] = 0;
		m_endEffectorTargetOrientation[1] = 0;
		m_endEffectorTargetOrientation[2] = 0;
		m_endEffectorTargetOrientation[3] = 1;
	}
};

struct RobotSimulatorInverseKinematicsResults
{
	i32 m_bodyUniqueId;
	AlignedObjectArray<double> m_calculatedJointPositions;
};

struct b3JointStates2
{
	i32 m_bodyUniqueId;
	i32 m_numDegreeOfFreedomQ;
	i32 m_numDegreeOfFreedomU;
	Transform2 m_rootLocalInertialFrame;
	AlignedObjectArray<double> m_actualStateQ;
	AlignedObjectArray<double> m_actualStateQdot;
	AlignedObjectArray<double> m_jointReactionForces;
};

struct RobotSimulatorJointMotorArrayArgs
{
	i32 m_controlMode;
	i32 m_numControlledDofs;

	i32 *m_jointIndices;

	double *m_targetPositions;
	double *m_kps;

	double *m_targetVelocities;
	double *m_kds;

	double *m_forces;

	RobotSimulatorJointMotorArrayArgs(i32 controlMode, i32 numControlledDofs)
		: m_controlMode(controlMode),
		  m_numControlledDofs(numControlledDofs),
		  m_jointIndices(NULL),
		  m_targetPositions(NULL),
		  m_kps(NULL),
		  m_targetVelocities(NULL),
		  m_kds(NULL),
		  m_forces(NULL)
	{
	}
};

struct RobotSimulatorGetCameraImageArgs
{
	i32 m_width;
	i32 m_height;
	float *m_viewMatrix;
	float *m_projectionMatrix;
	float *m_lightDirection;
	float *m_lightColor;
	float m_lightDistance;
	i32 m_hasShadow;
	float m_lightAmbientCoeff;
	float m_lightDiffuseCoeff;
	float m_lightSpecularCoeff;
	i32 m_renderer;

	RobotSimulatorGetCameraImageArgs(i32 width, i32 height)
		: m_width(width),
		  m_height(height),
		  m_viewMatrix(NULL),
		  m_projectionMatrix(NULL),
		  m_lightDirection(NULL),
		  m_lightColor(NULL),
		  m_lightDistance(-1),
		  m_hasShadow(-1),
		  m_lightAmbientCoeff(-1),
		  m_lightDiffuseCoeff(-1),
		  m_lightSpecularCoeff(-1),
		  m_renderer(-1)
	{
	}
};

struct RobotSimulatorSetPhysicsEngineParameters : b3PhysicsSimulationParameters
{
	RobotSimulatorSetPhysicsEngineParameters()
	{
		m_deltaTime = -1;
		m_gravityAcceleration[0] = 0;
		m_gravityAcceleration[1] = 0;
		m_gravityAcceleration[2] = 0;

		m_numSimulationSubSteps = -1;
		m_numSolverIterations = -1;
		m_useRealTimeSimulation = -1;
		m_useSplitImpulse = -1;
		m_splitImpulsePenetrationThreshold = -1;
		m_contactBreakingThreshold = -1;
		m_internalSimFlags = -1;
		m_defaultContactERP = -1;
		m_collisionFilterMode = -1;
		m_enableFileCaching = -1;
		m_restitutionVelocityThreshold = -1;
		m_defaultNonContactERP = -1;
		m_frictionERP = -1;
		m_defaultGlobalCFM = -1;
		m_frictionCFM = -1;
		m_enableConeFriction = -1;
		m_deterministicOverlappingPairs = -1;
		m_allowedCcdPenetration = -1;
		m_jointFeedbackMode = -1;
		m_solverResidualThreshold = -1;
		m_contactSlop = -1;

		m_collisionFilterMode = -1;
		m_contactBreakingThreshold = -1;

		m_enableFileCaching = -1;
		m_restitutionVelocityThreshold = -1;

		m_frictionERP = -1;
		m_solverResidualThreshold = -1;
		m_constraintSolverType = -1;
		m_minimumSolverIslandSize = -1;
	}
};

struct RobotSimulatorChangeDynamicsArgs
{
	double m_mass;
	double m_lateralFriction;
	double m_spinningFriction;
	double m_rollingFriction;
	double m_restitution;
	double m_linearDamping;
	double m_angularDamping;
	double m_contactStiffness;
	double m_contactDamping;
	i32 m_frictionAnchor;
	i32 m_activationState;

	RobotSimulatorChangeDynamicsArgs()
		: m_mass(-1),
		  m_lateralFriction(-1),
		  m_spinningFriction(-1),
		  m_rollingFriction(-1),
		  m_restitution(-1),
		  m_linearDamping(-1),
		  m_angularDamping(-1),
		  m_contactStiffness(-1),
		  m_contactDamping(-1),
		  m_frictionAnchor(-1),
		  m_activationState(-1)
	{
	}
};

struct RobotSimulatorAddUserDebugLineArgs
{
	double m_colorRGB[3];
	double m_lineWidth;
	double m_lifeTime;
	i32 m_parentObjectUniqueId;
	i32 m_parentLinkIndex;

	RobotSimulatorAddUserDebugLineArgs()
		: m_lineWidth(1),
		  m_lifeTime(0),
		  m_parentObjectUniqueId(-1),
		  m_parentLinkIndex(-1)
	{
		m_colorRGB[0] = 1;
		m_colorRGB[1] = 1;
		m_colorRGB[2] = 1;
	}
};

enum b3AddUserDebugTextFlags
{
	DEBUG_TEXT_HAS_ORIENTATION = 1
};

struct RobotSimulatorAddUserDebugTextArgs
{
	double m_colorRGB[3];
	double m_size;
	double m_lifeTime;
	double m_textOrientation[4];
	i32 m_parentObjectUniqueId;
	i32 m_parentLinkIndex;
	i32 m_flags;

	RobotSimulatorAddUserDebugTextArgs()
		: m_size(1),
		  m_lifeTime(0),
		  m_parentObjectUniqueId(-1),
		  m_parentLinkIndex(-1),
		  m_flags(0)
	{
		m_colorRGB[0] = 1;
		m_colorRGB[1] = 1;
		m_colorRGB[2] = 1;

		m_textOrientation[0] = 0;
		m_textOrientation[1] = 0;
		m_textOrientation[2] = 0;
		m_textOrientation[3] = 1;
	}
};

struct RobotSimulatorGetContactPointsArgs
{
	i32 m_bodyUniqueIdA;
	i32 m_bodyUniqueIdB;
	i32 m_linkIndexA;
	i32 m_linkIndexB;

	RobotSimulatorGetContactPointsArgs()
		: m_bodyUniqueIdA(-1),
		  m_bodyUniqueIdB(-1),
		  m_linkIndexA(-2),
		  m_linkIndexB(-2)
	{
	}
};

struct RobotSimulatorCreateCollisionShapeArgs
{
	i32 m_shapeType;
	double m_radius;
	Vec3 m_halfExtents;
	double m_height;
	char *m_fileName;
	Vec3 m_meshScale;
	Vec3 m_planeNormal;
	i32 m_flags;

	double m_heightfieldTextureScaling;
	AlignedObjectArray<float> m_heightfieldData;
	i32 m_numHeightfieldRows;
	i32 m_numHeightfieldColumns;
	i32 m_replaceHeightfieldIndex;

	RobotSimulatorCreateCollisionShapeArgs()
		: m_shapeType(-1),
		  m_radius(0.5),
		  m_height(1),
		  m_fileName(NULL),
		  m_flags(0),
		  m_heightfieldTextureScaling(1),
		  m_numHeightfieldRows(0),
		  m_numHeightfieldColumns(0),
		  m_replaceHeightfieldIndex(-1)
	{
		m_halfExtents.m_floats[0] = 1;
		m_halfExtents.m_floats[1] = 1;
		m_halfExtents.m_floats[2] = 1;

		m_meshScale.m_floats[0] = 1;
		m_meshScale.m_floats[1] = 1;
		m_meshScale.m_floats[2] = 1;

		m_planeNormal.m_floats[0] = 0;
		m_planeNormal.m_floats[1] = 0;
		m_planeNormal.m_floats[2] = 1;
	}
};


struct RobotSimulatorCreateVisualShapeArgs
{
	i32 m_shapeType;
	double m_radius;
	Vec3 m_halfExtents;
	double m_height;
	tuk m_fileName;
	Vec3 m_meshScale;
	Vec3 m_planeNormal;
	i32 m_flags;
	RobotSimulatorCreateVisualShapeArgs()
		: m_shapeType(-1),
		m_radius(0.5),
		m_height(1),
		m_fileName(NULL),
		m_flags(0)
	{
		m_halfExtents.m_floats[0] = 1;
		m_halfExtents.m_floats[1] = 1;
		m_halfExtents.m_floats[2] = 1;

		m_meshScale.m_floats[0] = 1;
		m_meshScale.m_floats[1] = 1;
		m_meshScale.m_floats[2] = 1;

		m_planeNormal.m_floats[0] = 0;
		m_planeNormal.m_floats[1] = 0;
		m_planeNormal.m_floats[2] = 1;
	}
};

struct RobotSimulatorCreateMultiBodyArgs
{
	double m_baseMass;
	i32 m_baseCollisionShapeIndex;
	i32 m_baseVisualShapeIndex;
	Vec3 m_basePosition;
	Quat m_baseOrientation;
	Vec3 m_baseInertialFramePosition;
	Quat m_baseInertialFrameOrientation;

	i32 m_numLinks;
	double *m_linkMasses;
	i32 *m_linkCollisionShapeIndices;
	i32 *m_linkVisualShapeIndices;
	Vec3 *m_linkPositions;
	Quat *m_linkOrientations;
	Vec3 *m_linkInertialFramePositions;
	Quat *m_linkInertialFrameOrientations;
	i32 *m_linkParentIndices;
	i32 *m_linkJointTypes;
	Vec3 *m_linkJointAxes;
	AlignedObjectArray<Vec3> m_batchPositions;
	i32 m_useMaximalCoordinates;

	RobotSimulatorCreateMultiBodyArgs()
		: m_baseMass(0), m_baseCollisionShapeIndex(-1), m_baseVisualShapeIndex(-1), m_numLinks(0), m_linkMasses(NULL), m_linkCollisionShapeIndices(NULL), m_linkVisualShapeIndices(NULL), m_linkPositions(NULL), m_linkOrientations(NULL), m_linkInertialFramePositions(NULL), m_linkInertialFrameOrientations(NULL), m_linkParentIndices(NULL), m_linkJointTypes(NULL), m_linkJointAxes(NULL), m_useMaximalCoordinates(0)
	{
		m_basePosition.setVal(0, 0, 0);
		m_baseOrientation.setVal(0, 0, 0, 1);
		m_baseInertialFramePosition.setVal(0, 0, 0);
		m_baseInertialFrameOrientation.setVal(0, 0, 0, 1);
	}
};


struct b3RobotUserConstraint : public b3UserConstraint
{
	i32 m_userUpdateFlags;//see EnumUserConstraintFlags

	void setErp(double erp)
	{
		m_erp = erp;
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_ERP;
	}
	
	void setMaxAppliedForce(double maxForce)
	{
		m_maxAppliedForce = maxForce;
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_MAX_FORCE;
	}
	
	void setGearRatio(double gearRatio)
	{
		m_gearRatio = gearRatio;
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_GEAR_RATIO;
	}

	void setGearAuxLink(i32 link)
	{
		m_gearAuxLink = link;
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_GEAR_AUX_LINK;
	}

	void setRelativePositionTarget(double target)
	{
		m_relativePositionTarget = target;
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_RELATIVE_POSITION_TARGET;
	}

	void setChildPivot(double pivot[3])
	{
		m_childFrame[0] = pivot[0];
		m_childFrame[1] = pivot[1];
		m_childFrame[2] = pivot[2];
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_PIVOT_IN_B;
	}

	void setChildFrameOrientation(double orn[4])
	{
		m_childFrame[3] = orn[0];
		m_childFrame[4] = orn[1];
		m_childFrame[5] = orn[2];
		m_childFrame[6] = orn[3];
		m_userUpdateFlags |= USER_CONSTRAINT_CHANGE_FRAME_ORN_IN_B;
	}

	b3RobotUserConstraint()
		:m_userUpdateFlags(0)
	{
		m_parentBodyIndex = -1;
		m_parentJointIndex = -1;
		m_childBodyIndex = -1;
		m_childJointIndex = -1;
		//position
		m_parentFrame[0] = 0;
		m_parentFrame[1] = 0;
		m_parentFrame[2] = 0;
		//orientation quaternion [x,y,z,w]
		m_parentFrame[3] = 0;
		m_parentFrame[4] = 0;
		m_parentFrame[5] = 0;
		m_parentFrame[6] = 1;

		//position
		m_childFrame[0] = 0;
		m_childFrame[1] = 0;
		m_childFrame[2] = 0;
		//orientation quaternion [x,y,z,w]
		m_childFrame[3] = 0;
		m_childFrame[4] = 0;
		m_childFrame[5] = 0;
		m_childFrame[6] = 1;

		m_jointAxis[0] = 0;
		m_jointAxis[1] = 0;
		m_jointAxis[2] = 1;

		m_jointType = eFixedType;

		m_maxAppliedForce = 500;
		m_userConstraintUniqueId = -1;
		m_gearRatio = -1;
		m_gearAuxLink = -1;
		m_relativePositionTarget = 0;
		m_erp = 0;
	}
};

struct b3RobotJointInfo : public b3JointInfo
{
	b3RobotJointInfo()
	{
		m_linkName[0] = 0;
		m_jointName[0] = 0;
		m_jointType = eFixedType;
		m_qIndex = -1;
		m_uIndex = -1;
		m_jointIndex = -1;
		m_flags = 0;
		m_jointDamping = 0;
		m_jointFriction = 0;
		m_jointLowerLimit = 1;
		m_jointUpperLimit = -1;
		m_jointMaxForce = 500;
		m_jointMaxVelocity = 100;
		m_parentIndex = -1;

		//position
		m_parentFrame[0] = 0;
		m_parentFrame[1] = 0;
		m_parentFrame[2] = 0;
		//orientation quaternion [x,y,z,w]
		m_parentFrame[3] = 0;
		m_parentFrame[4] = 0;
		m_parentFrame[5] = 0;
		m_parentFrame[6] = 1;

		//position
		m_childFrame[0] = 0;
		m_childFrame[1] = 0;
		m_childFrame[2] = 0;
		//orientation quaternion [x,y,z,w]
		m_childFrame[3] = 0;
		m_childFrame[4] = 0;
		m_childFrame[5] = 0;
		m_childFrame[6] = 1;

		m_jointAxis[0] = 0;
		m_jointAxis[1] = 0;
		m_jointAxis[2] = 1;
	}
};

class RobotSimulatorClientAPI_NoDirect
{
protected:
	struct RobotSimulatorClientAPI_InternalData *m_data;

public:
	RobotSimulatorClientAPI_NoDirect();
	virtual ~RobotSimulatorClientAPI_NoDirect();

	//No 'connect', use setInternalData to bypass the connect method, pass an existing client
	virtual void setInternalData(struct RobotSimulatorClientAPI_InternalData *data);

	void disconnect();

	bool isConnected() const;

	void setTimeOut(double timeOutInSec);

	void syncBodies();

	void resetSimulation();
    
	void resetSimulation(i32 flag);

	Quat getQuatFromEuler(const Vec3 &rollPitchYaw);
	Vec3 getEulerFromQuat(const Quat &quat);

	i32 loadURDF(const STxt &fileName, const struct RobotSimulatorLoadUrdfFileArgs &args = RobotSimulatorLoadUrdfFileArgs());
	bool loadSDF(const STxt &fileName, RobotSimulatorLoadFileResults &results, const struct RobotSimulatorLoadSdfFileArgs &args = RobotSimulatorLoadSdfFileArgs());
	bool loadMJCF(const STxt &fileName, RobotSimulatorLoadFileResults &results);
	bool loadBullet(const STxt &fileName, RobotSimulatorLoadFileResults &results);
	bool saveBullet(const STxt &fileName);

	i32 loadTexture(const STxt &fileName);

	bool changeVisualShape(const struct RobotSimulatorChangeVisualShapeArgs &args);

	bool savePythonWorld(const STxt &fileName);

	bool getBodyInfo(i32 bodyUniqueId, struct b3BodyInfo *bodyInfo);

	bool getBasePositionAndOrientation(i32 bodyUniqueId, Vec3 &basePosition, Quat &baseOrientation) const;
	bool resetBasePositionAndOrientation(i32 bodyUniqueId, const Vec3 &basePosition, const Quat &baseOrientation);

	bool getBaseVelocity(i32 bodyUniqueId, Vec3 &baseLinearVelocity, Vec3 &baseAngularVelocity) const;
	bool resetBaseVelocity(i32 bodyUniqueId, const Vec3 &linearVelocity, const Vec3 &angularVelocity) const;

	i32 getNumJoints(i32 bodyUniqueId) const;

	bool getJointInfo(i32 bodyUniqueId, i32 jointIndex, b3JointInfo *jointInfo);

	i32 createConstraint(i32 parentBodyIndex, i32 parentJointIndex, i32 childBodyIndex, i32 childJointIndex, b3JointInfo *jointInfo);

	i32 changeConstraint(i32 constraintId, b3RobotUserConstraint *jointInfo);

	void removeConstraint(i32 constraintId);

	bool getConstraintInfo(i32 constraintUniqueId, struct b3UserConstraint &constraintInfo);

	bool getJointState(i32 bodyUniqueId, i32 jointIndex, struct b3JointSensorState *state);

	bool getJointStates(i32 bodyUniqueId, b3JointStates2 &state);

	bool resetJointState(i32 bodyUniqueId, i32 jointIndex, double targetValue);

	void setJointMotorControl(i32 bodyUniqueId, i32 jointIndex, const struct RobotSimulatorJointMotorArgs &args);

	bool setJointMotorControlArray(i32 bodyUniqueId, i32 controlMode, i32 numControlledDofs,
								   i32 *jointIndices, double *targetVelocities, double *targetPositions,
								   double *forces, double *kps, double *kds);

	void stepSimulation();

	bool canSubmitCommand() const;

	void setRealTimeSimulation(bool enableRealTimeSimulation);

	void setInternalSimFlags(i32 flags);

	void setGravity(const Vec3 &gravityAcceleration);

	void setTimeStep(double timeStepInSeconds);
	void setNumSimulationSubSteps(i32 numSubSteps);
	void setNumSolverIterations(i32 numIterations);
	void setContactBreakingThreshold(double threshold);

	i32 computeDofCount(i32 bodyUniqueId) const;
	
	bool calculateInverseKinematics(const struct RobotSimulatorInverseKinematicArgs &args, struct RobotSimulatorInverseKinematicsResults &results);

	i32 calculateMassMatrix(i32 bodyUniqueId, const double* jointPositions, i32 numJointPositions, double* massMatrix, i32 flags);
	
	bool getBodyJacobian(i32 bodyUniqueId, i32 linkIndex, const double *localPosition, const double *jointPositions, const double *jointVelocities, const double *jointAccelerations, double *linearJacobian, double *angularJacobian);

	void configureDebugVisualizer(enum b3ConfigureDebugVisualizerEnum flag, i32 enable);
	void resetDebugVisualizerCamera(double cameraDistance, double cameraPitch, double cameraYaw, const Vec3 &targetPos);

	i32 startStateLogging(b3StateLoggingType loggingType, const STxt &fileName, const AlignedObjectArray<i32> &objectUniqueIds = AlignedObjectArray<i32>(), i32 maxLogDof = -1);
	void stopStateLogging(i32 stateLoggerUniqueId);

	void getVREvents(b3VREventsData *vrEventsData, i32 deviceTypeFilter);
	void getKeyboardEvents(b3KeyboardEventsData *keyboardEventsData);

	void submitProfileTiming(const STxt &profileName);

	// JFC: added these 24 methods

	void getMouseEvents(b3MouseEventsData *mouseEventsData);

	bool getLinkState(i32 bodyUniqueId, i32 linkIndex, i32 computeLinkVelocity, i32 computeForwardKinematics, b3LinkState *linkState);

	bool getCameraImage(i32 width, i32 height, struct RobotSimulatorGetCameraImageArgs args, b3CameraImageData &imageData);

	bool calculatedrx3d_inverse(i32 bodyUniqueId, double *jointPositions, double *jointVelocities, double *jointAccelerations, double *jointForcesOutput);

	i32 getNumBodies() const;

	i32 getBodyUniqueId(i32 bodyId) const;

	bool removeBody(i32 bodyUniqueId);

	bool getDynamicsInfo(i32 bodyUniqueId, i32 linkIndex, b3DynamicsInfo *dynamicsInfo);

	bool changeDynamics(i32 bodyUniqueId, i32 linkIndex, struct RobotSimulatorChangeDynamicsArgs &args);

	i32 addUserDebugParameter(tukk paramName, double rangeMin, double rangeMax, double startValue);

	double readUserDebugParameter(i32 itemUniqueId);

	bool removeUserDebugItem(i32 itemUniqueId);

	i32 addUserDebugText(tukk text, double *textPosition, struct RobotSimulatorAddUserDebugTextArgs &args);

	i32 addUserDebugText(tukk text, Vec3 &textPosition, struct RobotSimulatorAddUserDebugTextArgs &args);

	i32 addUserDebugLine(double *fromXYZ, double *toXYZ, struct RobotSimulatorAddUserDebugLineArgs &args);

	i32 addUserDebugLine(Vec3 &fromXYZ, Vec3 &toXYZ, struct RobotSimulatorAddUserDebugLineArgs &args);

	bool setJointMotorControlArray(i32 bodyUniqueId, struct RobotSimulatorJointMotorArrayArgs &args);

	bool setPhysicsEngineParameter(const struct RobotSimulatorSetPhysicsEngineParameters &args);

	bool getPhysicsEngineParameters(struct RobotSimulatorSetPhysicsEngineParameters &args);

	bool applyExternalForce(i32 objectUniqueId, i32 linkIndex, double *force, double *position, i32 flags);

	bool applyExternalForce(i32 objectUniqueId, i32 linkIndex, Vec3 &force, Vec3 &position, i32 flags);

	bool applyExternalTorque(i32 objectUniqueId, i32 linkIndex, double *torque, i32 flags);

	bool applyExternalTorque(i32 objectUniqueId, i32 linkIndex, Vec3 &torque, i32 flags);

	bool enableJointForceTorqueSensor(i32 bodyUniqueId, i32 jointIndex, bool enable);

	bool getDebugVisualizerCamera(struct b3OpenGLVisualizerCameraInfo *cameraInfo);

	bool getContactPoints(struct RobotSimulatorGetContactPointsArgs &args, struct b3ContactInformation *contactInfo);

	bool getClosestPoints(struct RobotSimulatorGetContactPointsArgs &args, double distance, struct b3ContactInformation *contactInfo);

	bool getOverlappingObjects(double *aabbMin, double *aabbMax, struct b3AABBOverlapData *overlapData);

	bool getOverlappingObjects(Vec3 &aabbMin, Vec3 &aabbMax, struct b3AABBOverlapData *overlapData);

	bool getAABB(i32 bodyUniqueId, i32 linkIndex, double *aabbMin, double *aabbMax);

	bool getAABB(i32 bodyUniqueId, i32 linkIndex, Vec3 &aabbMin, Vec3 &aabbMax);

	i32 createVisualShape(i32 shapeType, struct RobotSimulatorCreateVisualShapeArgs& args);

	i32 createCollisionShape(i32 shapeType, struct RobotSimulatorCreateCollisionShapeArgs &args);

	i32 createMultiBody(struct RobotSimulatorCreateMultiBodyArgs &args);

	i32 getNumConstraints() const;

	i32 getConstraintUniqueId(i32 serialIndex);

	void loadSoftBody(const STxt &fileName, const struct RobotSimulatorLoadSoftBodyArgs &args);
    
	void loadDeformableBody(const STxt &fileName, const struct RobotSimulatorLoadDeformableBodyArgs &args);

	virtual void setGuiHelper(struct GUIHelperInterface *guiHelper);
	virtual struct GUIHelperInterface *getGuiHelper();

	bool getCollisionShapeData(i32 bodyUniqueId, i32 linkIndex, b3CollisionShapeInformation &collisionShapeInfo);

	bool getVisualShapeData(i32 bodyUniqueId, struct b3VisualShapeInformation &visualShapeInfo);

	i32 saveStateToMemory();
	void restoreStateFromMemory(i32 stateId);
	void removeState(i32 stateUniqueId);

	i32 getAPIVersion() const
	{
		return SHARED_MEMORY_MAGIC_NUMBER;
	}
	void setAdditionalSearchPath(const STxt &path);
    
    void setCollisionFilterGroupMask(i32 bodyUniqueIdA, i32 linkIndexA, i32 collisionFilterGroup, i32 collisionFilterMask);
};

#endif  //D3_ROBOT_SIMULATOR_CLIENT_API_NO_DIRECT_H
