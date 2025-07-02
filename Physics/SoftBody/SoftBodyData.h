#ifndef DRX3D_SOFTBODY_FLOAT_DATA
#define DRX3D_SOFTBODY_FLOAT_DATA

#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

struct SoftBodyMaterialData
{
	float m_linearStiffness;
	float m_angularStiffness;
	float m_volumeStiffness;
	i32 m_flags;
};

struct SoftBodyNodeData
{
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

struct SoftBodyLinkData
{
	SoftBodyMaterialData *m_material;
	i32 m_nodeIndices[2];  // Указатели на узлы
	float m_restLength;    // Остаток длины
	i32 m_bbending;        // Bending link
};

struct SoftBodyFaceData
{
	Vec3FloatData m_normal;  // Normal
	SoftBodyMaterialData *m_material;
	i32 m_nodeIndices[3];  // Node pointers
	float m_restArea;      // Rest area
};

struct SoftBodyTetraData
{
	Vec3FloatData m_c0[4];  // gradients
	SoftBodyMaterialData *m_material;
	i32 m_nodeIndices[4];  // Node pointers
	float m_restVolume;    // Rest volume
	float m_c1;            // (4*kVST)/(im0+im1+im2+im3)
	float m_c2;            // m_c1/sum(|g0..3|^2)
	i32 m_pad;
};

struct SoftRigidAnchorData
{
	Matrix3x3FloatData m_c0;        // Impulse matrix
	Vec3FloatData m_c1;          // Relative anchor
	Vec3FloatData m_localFrame;  // Anchor position in body space
	RigidBodyData *m_rigidBody;
	i32 m_nodeIndex;  // Node pointer
	float m_c2;       // ima*dt
};

struct SoftBodyConfigData
{
	i32 m_aeroModel;                         // Aerodynamic model (default: V_Point)
	float m_baumgarte;                       // Velocities correction factor (Baumgarte)
	float m_damping;                         // Damping coefficient [0,1]
	float m_drag;                            // Drag coefficient [0,+inf]
	float m_lift;                            // Lift coefficient [0,+inf]
	float m_pressure;                        // Pressure coefficient [-inf,+inf]
	float m_volume;                          // Volume conversation coefficient [0,+inf]
	float m_dynamicFriction;                 // Dynamic friction coefficient [0,1]
	float m_poseMatch;                       // Pose matching coefficient [0,1]
	float m_rigidContactHardness;            // Rigid contacts hardness [0,1]
	float m_kineticContactHardness;          // Kinetic contacts hardness [0,1]
	float m_softContactHardness;             // Soft contacts hardness [0,1]
	float m_anchorHardness;                  // Anchors hardness [0,1]
	float m_softRigidClusterHardness;        // Soft vs rigid hardness [0,1] (cluster only)
	float m_softKineticClusterHardness;      // Soft vs kinetic hardness [0,1] (cluster only)
	float m_softSoftClusterHardness;         // Soft vs soft hardness [0,1] (cluster only)
	float m_softRigidClusterImpulseSplit;    // Soft vs rigid impulse split [0,1] (cluster only)
	float m_softKineticClusterImpulseSplit;  // Soft vs rigid impulse split [0,1] (cluster only)
	float m_softSoftClusterImpulseSplit;     // Soft vs rigid impulse split [0,1] (cluster only)
	float m_maxVolume;                       // Maximum volume ratio for pose
	float m_timeScale;                       // Time scale
	i32 m_velocityIterations;                // Velocities solver iterations
	i32 m_positionIterations;                // Positions solver iterations
	i32 m_driftIterations;                   // Drift solver iterations
	i32 m_clusterIterations;                 // Cluster solver iterations
	i32 m_collisionFlags;                    // Collisions flags
};

struct SoftBodyPoseData
{
	Matrix3x3FloatData m_rot;    // Rotation
	Matrix3x3FloatData m_scale;  // Scale
	Matrix3x3FloatData m_aqq;    // Base scaling
	Vec3FloatData m_com;      // COM

	Vec3FloatData *m_positions;  // Reference positions
	float *m_weights;                 // Weights
	i32 m_numPositions;
	i32 m_numWeigts;

	i32 m_bvolume;       // Is valid
	i32 m_bframe;        // Is frame
	float m_restVolume;  // Rest volume
	i32 m_pad;
};

struct SoftBodyClusterData
{
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

enum SoftJointBodyType
{
	DRX3D_JOINT_SOFT_BODY_CLUSTER = 1,
	DRX3D_JOINT_RIGID_BODY,
	DRX3D_JOINT_COLLISION_OBJECT
};

struct SoftBodyJointData
{
	uk m_bodyA;
	uk m_bodyB;
	Vec3FloatData m_refs[2];
	float m_cfm;
	float m_erp;
	float m_split;
	i32 m_delete;
	Vec3FloatData m_relPosition[2];  //linear
	i32 m_bodyAtype;
	i32 m_bodyBtype;
	i32 m_jointType;
	i32 m_pad;
};

///do not change those serialization structures, it requires an updated sBulletDNAstr/sBulletDNAstr64
struct SoftBodyFloatData
{
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

#endif  //DRX3D_SOFTBODY_FLOAT_DATA
