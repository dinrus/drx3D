#ifndef _DRX3D_SOFT_BODY_H
#define _DRX3D_SOFT_BODY_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Physics/Dynamics/RigidBody.h>

#include <drx3D/Physics/Collision/Shapes/ConcaveShape.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionCreateFunc.h>
#include <drx3D/Physics/SoftBody/SparseSDF.h>
#include <drx3D/Physics/Collision/BroadPhase/Dbvt.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyLinkCollider.h>
#include <drx3D/Physics/Dynamics/MultiBody/MultiBodyConstraint.h>
//#ifdef DRX3D_USE_DOUBLE_PRECISION
//#define RigidBodyData	RigidBodyDoubleData
//#define RigidBodyDataName	"RigidBodyDoubleData"
//#else
#define SoftBodyData SoftBodyFloatData
#define SoftBodyDataName "SoftBodyFloatData"
static const Scalar OVERLAP_REDUCTION_FACTOR = 0.1;
static u64 seed = 243703;
//#endif //DRX3D_USE_DOUBLE_PRECISION

class BroadphaseInterface;
class Dispatcher;
class SoftBodySolver;

/* SoftBodyWorldInfo	*/
struct SoftBodyWorldInfo
{
	Scalar air_density;
	Scalar water_density;
	Scalar water_offset;
	Scalar m_maxDisplacement;
	Vec3 water_normal;
	BroadphaseInterface* m_broadphase;
	Dispatcher* m_dispatcher;
	Vec3 m_gravity;
	SparseSdf<3> m_sparsesdf;

	SoftBodyWorldInfo()
		: air_density((Scalar)1.2),
		  water_density(0),
		  water_offset(0),
		  m_maxDisplacement(1000.f),  //avoid soft body from 'exploding' so use some upper threshold of maximum motion that a node can travel per frame
		  water_normal(0, 0, 0),
		  m_broadphase(0),
		  m_dispatcher(0),
		  m_gravity(0, -10, 0)
	{
	}
};

///The SoftBody is an class to simulate cloth and volumetric soft bodies.
///There is two-way interaction between SoftBody and RigidBodyCollisionObject2.
class SoftBody : public CollisionObject2
{
public:
	AlignedObjectArray<const class CollisionObject2*> m_collisionDisabledObjects;

	// The solver object that handles this soft body
	SoftBodySolver* m_softBodySolver;

	//
	// Enumerations
	//

	///eAeroModel
	struct eAeroModel
	{
		enum _
		{
			V_Point,             ///Vertex normals are oriented toward velocity
			V_TwoSided,          ///Vertex normals are flipped to match velocity
			V_TwoSidedLiftDrag,  ///Vertex normals are flipped to match velocity and lift and drag forces are applied
			V_OneSided,          ///Vertex normals are taken as it is
			F_TwoSided,          ///Face normals are flipped to match velocity
			F_TwoSidedLiftDrag,  ///Face normals are flipped to match velocity and lift and drag forces are applied
			F_OneSided,          ///Face normals are taken as it is
			END
		};
	};

	///eVSolver : velocities solvers
	struct eVSolver
	{
		enum _
		{
			Linear,  ///Linear solver
			END
		};
	};

	///ePSolver : positions solvers
	struct ePSolver
	{
		enum _
		{
			Linear,     ///Linear solver
			Anchors,    ///Anchor solver
			RContacts,  ///Rigid contacts solver
			SContacts,  ///Soft contacts solver
			END
		};
	};

	///eSolverPresets
	struct eSolverPresets
	{
		enum _
		{
			Positions,
			Velocities,
			Default = Positions,
			END
		};
	};

	///eFeature
	struct eFeature
	{
		enum _
		{
			None,
			Node,
			Link,
			Face,
			Tetra,
			END
		};
	};

	typedef AlignedObjectArray<eVSolver::_> tVSolverArray;
	typedef AlignedObjectArray<ePSolver::_> tPSolverArray;

	//
	// Flags
	//

	///fCollision
	struct fCollision
	{
		enum _
		{
			RVSmask = 0x000f,  ///Rigid versus soft mask
			SDF_RS = 0x0001,   ///SDF based rigid vs soft
			CL_RS = 0x0002,    ///Cluster vs convex rigid vs soft
			SDF_RD = 0x0004,   ///rigid vs deformable

			SVSmask = 0x00f0,  ///Rigid versus soft mask
			VF_SS = 0x0010,    ///Vertex vs face soft vs soft handling
			CL_SS = 0x0020,    ///Cluster vs cluster soft vs soft handling
			CL_SELF = 0x0040,  ///Cluster soft body self collision
			VF_DD = 0x0080,    ///Vertex vs face soft vs soft handling

			RVDFmask = 0x0f00,  /// Rigid versus deformable face mask
			SDF_RDF = 0x0100,   /// GJK based Rigid vs. deformable face
			SDF_MDF = 0x0200,   /// GJK based Multibody vs. deformable face
			SDF_RDN = 0x0400,   /// SDF based Rigid vs. deformable node
			/* presets	*/
			Default = SDF_RS,
			END
		};
	};

	///fMaterial
	struct fMaterial
	{
		enum _
		{
			DebugDraw = 0x0001,  /// Enable debug draw
			/* presets	*/
			Default = DebugDraw,
			END
		};
	};

	//
	// API Types
	//

	/* sRayCast		*/
	struct sRayCast
	{
		SoftBody* body;     /// soft body
		eFeature::_ feature;  /// feature type
		i32 index;            /// feature index
		Scalar fraction;    /// time of impact fraction (rayorg+(rayto-rayfrom)*fraction)
	};

	/* ImplicitFn	*/
	struct ImplicitFn
	{
		virtual ~ImplicitFn() {}
		virtual Scalar Eval(const Vec3& x) = 0;
	};

	//
	// Internal types
	//

	typedef AlignedObjectArray<Scalar> tScalarArray;
	typedef AlignedObjectArray<Vec3> tVector3Array;

	/* sCti is Softbody contact info	*/
	struct sCti
	{
		const CollisionObject2* m_colObj; /* Rigid body			*/
		Vec3 m_normal;                /* Outward normal		*/
		Scalar m_offset;                 /* Offset from origin	*/
		Vec3 m_bary;                  /* Barycentric weights for faces */
	};

	/* sMedium		*/
	struct sMedium
	{
		Vec3 m_velocity; /* Velocity				*/
		Scalar m_pressure;  /* Pressure				*/
		Scalar m_density;   /* Density				*/
	};

	/* Base type	*/
	struct Element
	{
		uk m_tag;  // User data
		Element() : m_tag(0) {}
	};
	/* Material		*/
	struct Material : Element
	{
		Scalar m_kLST;  // Linear stiffness coefficient [0,1]
		Scalar m_kAST;  // Area/Angular stiffness coefficient [0,1]
		Scalar m_kVST;  // Volume stiffness coefficient [0,1]
		i32 m_flags;      // Flags
	};

	/* Feature		*/
	struct Feature : Element
	{
		Material* m_material;  // Material
	};
	/* Node			*/
	struct RenderNode
	{
		Vec3 m_x;
		Vec3 m_uv1;
		Vec3 m_normal;
	};
	struct Node : Feature
	{
		Vec3 m_x;       // Position
		Vec3 m_q;       // Previous step position/Test position
		Vec3 m_v;       // Velocity
		Vec3 m_vn;      // Previous step velocity
		Vec3 m_f;       // Force accumulator
		Vec3 m_n;       // Normal
		Scalar m_im;       // 1/mass
		Scalar m_area;     // Area
		DbvtNode* m_leaf;  // Leaf data
		i32 m_constrained;   // depth of penetration
		i32 m_battach : 1;   // Attached
		i32 index;
		Vec3 m_splitv;               // velocity associated with split impulse
		Matrix3x3 m_effectiveMass;      // effective mass in contact
		Matrix3x3 m_effectiveMass_inv;  // inverse of effective mass
	};
	/* Link			*/
	ATTRIBUTE_ALIGNED16(struct)
	Link : Feature
	{
		Vec3 m_c3;      // gradient
		Node* m_n[2];        // Node pointers
		Scalar m_rl;       // Rest length
		i32 m_bbending : 1;  // Bending link
		Scalar m_c0;       // (ima+imb)*kLST
		Scalar m_c1;       // rl^2
		Scalar m_c2;       // |gradient|^2/c0

		DRX3D_DECLARE_ALIGNED_ALLOCATOR();
	};
	struct RenderFace
	{
		RenderNode* m_n[3];  // Node pointers
	};

	/* Face			*/
	struct Face : Feature
	{
		Node* m_n[3];          // Node pointers
		Vec3 m_normal;    // Normal
		Scalar m_ra;         // Rest area
		DbvtNode* m_leaf;    // Leaf data
		Vec4 m_pcontact;  // barycentric weights of the persistent contact
		Vec3 m_n0, m_n1, m_vn;
		i32 m_index;
	};
	/* Tetra		*/
	struct Tetra : Feature
	{
		Node* m_n[4];              // Node pointers
		Scalar m_rv;             // Rest volume
		DbvtNode* m_leaf;        // Leaf data
		Vec3 m_c0[4];         // gradients
		Scalar m_c1;             // (4*kVST)/(im0+im1+im2+im3)
		Scalar m_c2;             // m_c1/sum(|g0..3|^2)
		Matrix3x3 m_Dm_inverse;  // rest Dm^-1
		Matrix3x3 m_F;
		Scalar m_element_measure;
		Vec4 m_P_inv[3];  // first three columns of P_inv matrix
	};

	/*  TetraScratch  */
	struct TetraScratch
	{
		Matrix3x3 m_F;           // deformation gradient F
		Scalar m_trace;          // trace of F^T * F
		Scalar m_J;              // det(F)
		Matrix3x3 m_cofF;        // cofactor of F
		Matrix3x3 m_corotation;  // corotatio of the tetra
	};

	/* RContact		*/
	struct RContact
	{
		sCti m_cti;        // Contact infos
		Node* m_node;      // Owner node
		Matrix3x3 m_c0;  // Impulse matrix
		Vec3 m_c1;    // Relative anchor
		Scalar m_c2;     // ima*dt
		Scalar m_c3;     // Friction
		Scalar m_c4;     // Hardness

		// jacobians and unit impulse responses for multibody
		MultiBodyJacobianData jacobianData_normal;
		MultiBodyJacobianData jacobianData_t1;
		MultiBodyJacobianData jacobianData_t2;
		Vec3 t1;
		Vec3 t2;
	};

	class DeformableRigidContact
	{
	public:
		sCti m_cti;        // Contact infos
		Matrix3x3 m_c0;  // Impulse matrix
		Vec3 m_c1;    // Relative anchor
		Scalar m_c2;     // inverse mass of node/face
		Scalar m_c3;     // Friction
		Scalar m_c4;     // Hardness
		Matrix3x3 m_c5;  // inverse effective mass

		// jacobians and unit impulse responses for multibody
		MultiBodyJacobianData jacobianData_normal;
		MultiBodyJacobianData jacobianData_t1;
		MultiBodyJacobianData jacobianData_t2;
		Vec3 t1;
		Vec3 t2;
	};

	class DeformableNodeRigidContact : public DeformableRigidContact
	{
	public:
		Node* m_node;  // Owner node
	};

	class DeformableNodeRigidAnchor : public DeformableNodeRigidContact
	{
	public:
		Vec3 m_local;  // Anchor position in body space
	};

	class DeformableFaceRigidContact : public DeformableRigidContact
	{
	public:
		Face* m_face;              // Owner face
		Vec3 m_contactPoint;  // Contact point
		Vec3 m_bary;          // Barycentric weights
		Vec3 m_weights;       // v_contactPoint * m_weights[i] = m_face->m_node[i]->m_v;
	};

	struct DeformableFaceNodeContact
	{
		Node* m_node;         // Node
		Face* m_face;         // Face
		Vec3 m_bary;     // Barycentric weights
		Vec3 m_weights;  // v_contactPoint * m_weights[i] = m_face->m_node[i]->m_v;
		Vec3 m_normal;   // Normal
		Scalar m_margin;    // Margin
		Scalar m_friction;  // Friction
		Scalar m_imf;       // inverse mass of the face at contact point
		Scalar m_c0;        // scale of the impulse matrix;
		const CollisionObject2* m_colObj;  // Collision object to collide with.
	};

	/* SContact		*/
	struct SContact
	{
		Node* m_node;         // Node
		Face* m_face;         // Face
		Vec3 m_weights;  // Weigths
		Vec3 m_normal;   // Normal
		Scalar m_margin;    // Margin
		Scalar m_friction;  // Friction
		Scalar m_cfm[2];    // Constraint force mixing
	};
	/* Anchor		*/
	struct Anchor
	{
		Node* m_node;         // Node pointer
		Vec3 m_local;    // Anchor position in body space
		RigidBody* m_body;  // Body
		Scalar m_influence;
		Matrix3x3 m_c0;  // Impulse matrix
		Vec3 m_c1;    // Relative anchor
		Scalar m_c2;     // ima*dt
	};
	/* Note			*/
	struct Note : Element
	{
		tukk m_text;    // Text
		Vec3 m_offset;    // Offset
		i32 m_rank;            // Rank
		Node* m_nodes[4];      // Nodes
		Scalar m_coords[4];  // Coordinates
	};
	/* Pose			*/
	struct Pose
	{
		bool m_bvolume;       // Is valid
		bool m_bframe;        // Is frame
		Scalar m_volume;    // Rest volume
		tVector3Array m_pos;  // Reference positions
		tScalarArray m_wgh;   // Weights
		Vec3 m_com;      // COM
		Matrix3x3 m_rot;    // Rotation
		Matrix3x3 m_scl;    // Scale
		Matrix3x3 m_aqq;    // Base scaling
	};
	/* Cluster		*/
	struct Cluster
	{
		tScalarArray m_masses;
		AlignedObjectArray<Node*> m_nodes;
		tVector3Array m_framerefs;
		Transform2 m_framexform;
		Scalar m_idmass;
		Scalar m_imass;
		Matrix3x3 m_locii;
		Matrix3x3 m_invwi;
		Vec3 m_com;
		Vec3 m_vimpulses[2];
		Vec3 m_dimpulses[2];
		i32 m_nvimpulses;
		i32 m_ndimpulses;
		Vec3 m_lv;
		Vec3 m_av;
		DbvtNode* m_leaf;
		Scalar m_ndamping; /* Node damping		*/
		Scalar m_ldamping; /* Linear damping	*/
		Scalar m_adamping; /* Angular damping	*/
		Scalar m_matching;
		Scalar m_maxSelfCollisionImpulse;
		Scalar m_selfCollisionImpulseFactor;
		bool m_containsAnchor;
		bool m_collide;
		i32 m_clusterIndex;
		Cluster() : m_leaf(0), m_ndamping(0), m_ldamping(0), m_adamping(0), m_matching(0), m_maxSelfCollisionImpulse(100.f), m_selfCollisionImpulseFactor(0.01f), m_containsAnchor(false)
		{
		}
	};
	/* Impulse		*/
	struct Impulse
	{
		Vec3 m_velocity;
		Vec3 m_drift;
		i32 m_asVelocity : 1;
		i32 m_asDrift : 1;
		Impulse() : m_velocity(0, 0, 0), m_drift(0, 0, 0), m_asVelocity(0), m_asDrift(0) {}
		Impulse operator-() const
		{
			Impulse i = *this;
			i.m_velocity = -i.m_velocity;
			i.m_drift = -i.m_drift;
			return (i);
		}
		Impulse operator*(Scalar x) const
		{
			Impulse i = *this;
			i.m_velocity *= x;
			i.m_drift *= x;
			return (i);
		}
	};
	/* Body			*/
	struct Body
	{
		Cluster* m_soft;
		RigidBody* m_rigid;
		const CollisionObject2* m_collisionObject;

		Body() : m_soft(0), m_rigid(0), m_collisionObject(0) {}
		Body(Cluster* p) : m_soft(p), m_rigid(0), m_collisionObject(0) {}
		Body(const CollisionObject2* colObj) : m_soft(0), m_collisionObject(colObj)
		{
			m_rigid = (RigidBody*)RigidBody::upcast(m_collisionObject);
		}

		void activate() const
		{
			if (m_rigid)
				m_rigid->activate();
			if (m_collisionObject)
				m_collisionObject->activate();
		}
		const Matrix3x3& invWorldInertia() const
		{
			static const Matrix3x3 iwi(0, 0, 0, 0, 0, 0, 0, 0, 0);
			if (m_rigid) return (m_rigid->getInvInertiaTensorWorld());
			if (m_soft) return (m_soft->m_invwi);
			return (iwi);
		}
		Scalar invMass() const
		{
			if (m_rigid) return (m_rigid->getInvMass());
			if (m_soft) return (m_soft->m_imass);
			return (0);
		}
		const Transform2& xform() const
		{
			static const Transform2 identity = Transform2::getIdentity();
			if (m_collisionObject) return (m_collisionObject->getWorldTransform());
			if (m_soft) return (m_soft->m_framexform);
			return (identity);
		}
		Vec3 linearVelocity() const
		{
			if (m_rigid) return (m_rigid->getLinearVelocity());
			if (m_soft) return (m_soft->m_lv);
			return (Vec3(0, 0, 0));
		}
		Vec3 angularVelocity(const Vec3& rpos) const
		{
			if (m_rigid) return (Cross(m_rigid->getAngularVelocity(), rpos));
			if (m_soft) return (Cross(m_soft->m_av, rpos));
			return (Vec3(0, 0, 0));
		}
		Vec3 angularVelocity() const
		{
			if (m_rigid) return (m_rigid->getAngularVelocity());
			if (m_soft) return (m_soft->m_av);
			return (Vec3(0, 0, 0));
		}
		Vec3 velocity(const Vec3& rpos) const
		{
			return (linearVelocity() + angularVelocity(rpos));
		}
		void applyVImpulse(const Vec3& impulse, const Vec3& rpos) const
		{
			if (m_rigid) m_rigid->applyImpulse(impulse, rpos);
			if (m_soft) SoftBody::clusterVImpulse(m_soft, rpos, impulse);
		}
		void applyDImpulse(const Vec3& impulse, const Vec3& rpos) const
		{
			if (m_rigid) m_rigid->applyImpulse(impulse, rpos);
			if (m_soft) SoftBody::clusterDImpulse(m_soft, rpos, impulse);
		}
		void applyImpulse(const Impulse& impulse, const Vec3& rpos) const
		{
			if (impulse.m_asVelocity)
			{
				//				printf("impulse.m_velocity = %f,%f,%f\n",impulse.m_velocity.getX(),impulse.m_velocity.getY(),impulse.m_velocity.getZ());
				applyVImpulse(impulse.m_velocity, rpos);
			}
			if (impulse.m_asDrift)
			{
				//				printf("impulse.m_drift = %f,%f,%f\n",impulse.m_drift.getX(),impulse.m_drift.getY(),impulse.m_drift.getZ());
				applyDImpulse(impulse.m_drift, rpos);
			}
		}
		void applyVAImpulse(const Vec3& impulse) const
		{
			if (m_rigid) m_rigid->applyTorqueImpulse(impulse);
			if (m_soft) SoftBody::clusterVAImpulse(m_soft, impulse);
		}
		void applyDAImpulse(const Vec3& impulse) const
		{
			if (m_rigid) m_rigid->applyTorqueImpulse(impulse);
			if (m_soft) SoftBody::clusterDAImpulse(m_soft, impulse);
		}
		void applyAImpulse(const Impulse& impulse) const
		{
			if (impulse.m_asVelocity) applyVAImpulse(impulse.m_velocity);
			if (impulse.m_asDrift) applyDAImpulse(impulse.m_drift);
		}
		void applyDCImpulse(const Vec3& impulse) const
		{
			if (m_rigid) m_rigid->applyCentralImpulse(impulse);
			if (m_soft) SoftBody::clusterDCImpulse(m_soft, impulse);
		}
	};
	/* Joint		*/
	struct Joint
	{
		struct eType
		{
			enum _
			{
				Linear = 0,
				Angular,
				Contact
			};
		};
		struct Specs
		{
			Specs() : erp(1), cfm(1), split(1) {}
			Scalar erp;
			Scalar cfm;
			Scalar split;
		};
		Body m_bodies[2];
		Vec3 m_refs[2];
		Scalar m_cfm;
		Scalar m_erp;
		Scalar m_split;
		Vec3 m_drift;
		Vec3 m_sdrift;
		Matrix3x3 m_massmatrix;
		bool m_delete;
		virtual ~Joint() {}
		Joint() : m_delete(false) {}
		virtual void Prepare(Scalar dt, i32 iterations);
		virtual void Solve(Scalar dt, Scalar sor) = 0;
		virtual void Terminate(Scalar dt) = 0;
		virtual eType::_ Type() const = 0;
	};
	/* LJoint		*/
	struct LJoint : Joint
	{
		struct Specs : Joint::Specs
		{
			Vec3 position;
		};
		Vec3 m_rpos[2];
		void Prepare(Scalar dt, i32 iterations);
		void Solve(Scalar dt, Scalar sor);
		void Terminate(Scalar dt);
		eType::_ Type() const { return (eType::Linear); }
	};
	/* AJoint		*/
	struct AJoint : Joint
	{
		struct IControl
		{
			virtual ~IControl() {}
			virtual void Prepare(AJoint*) {}
			virtual Scalar Speed(AJoint*, Scalar current) { return (current); }
			static IControl* Default()
			{
				static IControl def;
				return (&def);
			}
		};
		struct Specs : Joint::Specs
		{
			Specs() : icontrol(IControl::Default()) {}
			Vec3 axis;
			IControl* icontrol;
		};
		Vec3 m_axis[2];
		IControl* m_icontrol;
		void Prepare(Scalar dt, i32 iterations);
		void Solve(Scalar dt, Scalar sor);
		void Terminate(Scalar dt);
		eType::_ Type() const { return (eType::Angular); }
	};
	/* CJoint		*/
	struct CJoint : Joint
	{
		i32 m_life;
		i32 m_maxlife;
		Vec3 m_rpos[2];
		Vec3 m_normal;
		Scalar m_friction;
		void Prepare(Scalar dt, i32 iterations);
		void Solve(Scalar dt, Scalar sor);
		void Terminate(Scalar dt);
		eType::_ Type() const { return (eType::Contact); }
	};
	/* Config		*/
	struct Config
	{
		eAeroModel::_ aeromodel;    // Aerodynamic model (default: V_Point)
		Scalar kVCF;              // Velocities correction factor (Baumgarte)
		Scalar kDP;               // Damping coefficient [0,1]
		Scalar kDG;               // Drag coefficient [0,+inf]
		Scalar kLF;               // Lift coefficient [0,+inf]
		Scalar kPR;               // Pressure coefficient [-inf,+inf]
		Scalar kVC;               // Volume conversation coefficient [0,+inf]
		Scalar kDF;               // Dynamic friction coefficient [0,1]
		Scalar kMT;               // Pose matching coefficient [0,1]
		Scalar kCHR;              // Rigid contacts hardness [0,1]
		Scalar kKHR;              // Kinetic contacts hardness [0,1]
		Scalar kSHR;              // Soft contacts hardness [0,1]
		Scalar kAHR;              // Anchors hardness [0,1]
		Scalar kSRHR_CL;          // Soft vs rigid hardness [0,1] (cluster only)
		Scalar kSKHR_CL;          // Soft vs kinetic hardness [0,1] (cluster only)
		Scalar kSSHR_CL;          // Soft vs soft hardness [0,1] (cluster only)
		Scalar kSR_SPLT_CL;       // Soft vs rigid impulse split [0,1] (cluster only)
		Scalar kSK_SPLT_CL;       // Soft vs rigid impulse split [0,1] (cluster only)
		Scalar kSS_SPLT_CL;       // Soft vs rigid impulse split [0,1] (cluster only)
		Scalar maxvolume;         // Maximum volume ratio for pose
		Scalar timescale;         // Time scale
		i32 viterations;            // Velocities solver iterations
		i32 piterations;            // Positions solver iterations
		i32 diterations;            // Drift solver iterations
		i32 citerations;            // Cluster solver iterations
		i32 collisions;             // Collisions flags
		tVSolverArray m_vsequence;  // Velocity solvers sequence
		tPSolverArray m_psequence;  // Position solvers sequence
		tPSolverArray m_dsequence;  // Drift solvers sequence
		Scalar drag;              // deformable air drag
		Scalar m_maxStress;       // Maximum principle first Piola stress
	};
	/* SolverState	*/
	struct SolverState
	{
		//if you add new variables, always initialize them!
		SolverState()
			: sdt(0),
			  isdt(0),
			  velmrg(0),
			  radmrg(0),
			  updmrg(0)
		{
		}
		Scalar sdt;     // dt*timescale
		Scalar isdt;    // 1/sdt
		Scalar velmrg;  // velocity margin
		Scalar radmrg;  // radial margin
		Scalar updmrg;  // Update margin
	};
	/// RayFromToCaster takes a ray from, ray to (instead of direction!)
	struct RayFromToCaster : Dbvt::ICollide
	{
		Vec3 m_rayFrom;
		Vec3 m_rayTo;
		Vec3 m_rayNormalizedDirection;
		Scalar m_mint;
		Face* m_face;
		i32 m_tests;
		RayFromToCaster(const Vec3& rayFrom, const Vec3& rayTo, Scalar mxt);
		void Process(const DbvtNode* leaf);

		static /*inline*/ Scalar rayFromToTriangle(const Vec3& rayFrom,
													 const Vec3& rayTo,
													 const Vec3& rayNormalizedDirection,
													 const Vec3& a,
													 const Vec3& b,
													 const Vec3& c,
													 Scalar maxt = SIMD_INFINITY);
	};

	//
	// Typedefs
	//

	typedef void (*psolver_t)(SoftBody*, Scalar, Scalar);
	typedef void (*vsolver_t)(SoftBody*, Scalar);
	typedef AlignedObjectArray<Cluster*> tClusterArray;
	typedef AlignedObjectArray<Note> tNoteArray;
	typedef AlignedObjectArray<Node> tNodeArray;
	typedef AlignedObjectArray<RenderNode> tRenderNodeArray;
	typedef AlignedObjectArray<DbvtNode*> tLeafArray;
	typedef AlignedObjectArray<Link> tLinkArray;
	typedef AlignedObjectArray<Face> tFaceArray;
	typedef AlignedObjectArray<RenderFace> tRenderFaceArray;
	typedef AlignedObjectArray<Tetra> tTetraArray;
	typedef AlignedObjectArray<Anchor> tAnchorArray;
	typedef AlignedObjectArray<RContact> tRContactArray;
	typedef AlignedObjectArray<SContact> tSContactArray;
	typedef AlignedObjectArray<Material*> tMaterialArray;
	typedef AlignedObjectArray<Joint*> tJointArray;
	typedef AlignedObjectArray<SoftBody*> tSoftBodyArray;
	typedef AlignedObjectArray<AlignedObjectArray<Scalar> > tDenseMatrix;

	//
	// Fields
	//

	Config m_cfg;                      // Configuration
	SolverState m_sst;                 // Solver state
	Pose m_pose;                       // Pose
	uk m_tag;                       // User data
	SoftBodyWorldInfo* m_worldInfo;  // World info
	tNoteArray m_notes;                // Notes
	tNodeArray m_nodes;                // Nodes
	tRenderNodeArray m_renderNodes;    // Render Nodes
	tLinkArray m_links;                // Links
	tFaceArray m_faces;                // Faces
	tRenderFaceArray m_renderFaces;    // Faces
	tTetraArray m_tetras;              // Tetras
	AlignedObjectArray<TetraScratch> m_tetraScratches;
	AlignedObjectArray<TetraScratch> m_tetraScratchesTn;
	tAnchorArray m_anchors;  // Anchors
	AlignedObjectArray<DeformableNodeRigidAnchor> m_deformableAnchors;
	tRContactArray m_rcontacts;  // Rigid contacts
	AlignedObjectArray<DeformableNodeRigidContact> m_nodeRigidContacts;
	AlignedObjectArray<DeformableFaceNodeContact> m_faceNodeContacts;
	AlignedObjectArray<DeformableFaceRigidContact> m_faceRigidContacts;
	AlignedObjectArray<DeformableFaceNodeContact> m_faceNodeContactsCCD;
	tSContactArray m_scontacts;     // Soft contacts
	tJointArray m_joints;           // Joints
	tMaterialArray m_materials;     // Materials
	Scalar m_timeacc;             // Time accumulator
	Vec3 m_bounds[2];          // Spatial bounds
	bool m_bUpdateRtCst;            // Update runtime constants
	Dbvt m_ndbvt;                 // Nodes tree
	Dbvt m_fdbvt;                 // Faces tree
	DbvntNode* m_fdbvnt;          // Faces tree with normals
	Dbvt m_cdbvt;                 // Clusters tree
	tClusterArray m_clusters;       // Clusters
	Scalar m_dampingCoefficient;  // Damping Coefficient
	Scalar m_sleepingThreshold;
	Scalar m_maxSpeedSquared;
	AlignedObjectArray<Vec3> m_quads;  // quadrature points for collision detection
	Scalar m_repulsionStiffness;
	Scalar m_gravityFactor;
	bool m_cacheBarycenter;
	AlignedObjectArray<Vec3> m_X;  // initial positions

	AlignedObjectArray<Vec4> m_renderNodesInterpolationWeights;
	AlignedObjectArray<AlignedObjectArray<const SoftBody::Node*> > m_renderNodesParents;
	AlignedObjectArray<Scalar> m_z;  // vertical distance used in extrapolation
	bool m_useSelfCollision;
	bool m_softSoftCollision;

	AlignedObjectArray<bool> m_clusterConnectivity;  //cluster connectivity, for self-collision

	Vec3 m_windVelocity;

	Scalar m_restLengthScale;

	bool m_reducedModel;	// Reduced deformable model flag
	
	//
	// Api
	//

	/* ctor																	*/
	SoftBody(SoftBodyWorldInfo* worldInfo, i32 node_count, const Vec3* x, const Scalar* m);

	/* ctor																	*/
	SoftBody(SoftBodyWorldInfo* worldInfo);

	void initDefaults();

	/* dtor																	*/
	virtual ~SoftBody();
	/* Check for existing link												*/

	AlignedObjectArray<i32> m_userIndexMapping;

	SoftBodyWorldInfo* getWorldInfo()
	{
		return m_worldInfo;
	}

	void setDampingCoefficient(Scalar damping_coeff)
	{
		m_dampingCoefficient = damping_coeff;
	}

	///@todo: avoid internal softbody shape hack and move collision code to collision library
	virtual void setCollisionShape(CollisionShape* collisionShape)
	{
	}

	bool checkLink(i32 node0,
				   i32 node1) const;
	bool checkLink(const Node* node0,
				   const Node* node1) const;
	/* Check for existring face												*/
	bool checkFace(i32 node0,
				   i32 node1,
				   i32 node2) const;
	/* Append material														*/
	Material* appendMaterial();
	/* Append note															*/
	void appendNote(tukk text,
					const Vec3& o,
					const Vec4& c = Vec4(1, 0, 0, 0),
					Node* n0 = 0,
					Node* n1 = 0,
					Node* n2 = 0,
					Node* n3 = 0);
	void appendNote(tukk text,
					const Vec3& o,
					Node* feature);
	void appendNote(tukk text,
					const Vec3& o,
					Link* feature);
	void appendNote(tukk text,
					const Vec3& o,
					Face* feature);
	/* Append node															*/
	void appendNode(const Vec3& x, Scalar m);
	/* Append link															*/
	void appendLink(i32 model = -1, Material* mat = 0);
	void appendLink(i32 node0,
					i32 node1,
					Material* mat = 0,
					bool bcheckexist = false);
	void appendLink(Node* node0,
					Node* node1,
					Material* mat = 0,
					bool bcheckexist = false);
	/* Append face															*/
	void appendFace(i32 model = -1, Material* mat = 0);
	void appendFace(i32 node0,
					i32 node1,
					i32 node2,
					Material* mat = 0);
	void appendTetra(i32 model, Material* mat);
	//
	void appendTetra(i32 node0,
					 i32 node1,
					 i32 node2,
					 i32 node3,
					 Material* mat = 0);

	/* Append anchor														*/
	void appendDeformableAnchor(i32 node, RigidBody* body);
	void appendDeformableAnchor(i32 node, MultiBodyLinkCollider* link);
	void appendAnchor(i32 node,
					  RigidBody* body, bool disableCollisionBetweenLinkedBodies = false, Scalar influence = 1);
	void appendAnchor(i32 node, RigidBody* body, const Vec3& localPivot, bool disableCollisionBetweenLinkedBodies = false, Scalar influence = 1);
	void removeAnchor(i32 node);
	/* Append linear joint													*/
	void appendLinearJoint(const LJoint::Specs& specs, Cluster* body0, Body body1);
	void appendLinearJoint(const LJoint::Specs& specs, Body body = Body());
	void appendLinearJoint(const LJoint::Specs& specs, SoftBody* body);
	/* Append linear joint													*/
	void appendAngularJoint(const AJoint::Specs& specs, Cluster* body0, Body body1);
	void appendAngularJoint(const AJoint::Specs& specs, Body body = Body());
	void appendAngularJoint(const AJoint::Specs& specs, SoftBody* body);
	/* Add force (or gravity) to the entire body							*/
	void addForce(const Vec3& force);
	/* Add force (or gravity) to a node of the body							*/
	void addForce(const Vec3& force,
				  i32 node);
	/* Add aero force to a node of the body */
	void addAeroForceToNode(const Vec3& windVelocity, i32 nodeIndex);

	/* Add aero force to a face of the body */
	void addAeroForceToFace(const Vec3& windVelocity, i32 faceIndex);

	/* Add velocity to the entire body										*/
	void addVelocity(const Vec3& velocity);

	/* Set velocity for the entire body										*/
	void setVelocity(const Vec3& velocity);

	/* Add velocity to a node of the body									*/
	void addVelocity(const Vec3& velocity,
					 i32 node);
	/* Set mass																*/
	void setMass(i32 node,
				 Scalar mass);
	/* Get mass																*/
	Scalar getMass(i32 node) const;
	/* Get total mass														*/
	Scalar getTotalMass() const;
	/* Set total mass (weighted by previous masses)							*/
	void setTotalMass(Scalar mass,
					  bool fromfaces = false);
	/* Set total density													*/
	void setTotalDensity(Scalar density);
	/* Set volume mass (using tetrahedrons)									*/
	void setVolumeMass(Scalar mass);
	/* Set volume density (using tetrahedrons)								*/
	void setVolumeDensity(Scalar density);
	/* Get the linear velocity of the center of mass                        */
	Vec3 getLinearVelocity();
	/* Set the linear velocity of the center of mass                        */
	void setLinearVelocity(const Vec3& linVel);
	/* Set the angular velocity of the center of mass                       */
	void setAngularVelocity(const Vec3& angVel);
	/* Get best fit rigid transform                                         */
	Transform2 getRigidTransform();
	/* Transform2 to given pose                                              */
	virtual void transformTo(const Transform2& trs);
	/* Transform2															*/
	virtual void transform(const Transform2& trs);
	/* Translate															*/
	virtual void translate(const Vec3& trs);
	/* Rotate															*/
	virtual void rotate(const Quat& rot);
	/* Scale																*/
	virtual void scale(const Vec3& scl);
	/* Get link resting lengths scale										*/
	Scalar getRestLengthScale();
	/* Scale resting length of all springs									*/
	void setRestLengthScale(Scalar restLength);
	/* Set current state as pose											*/
	void setPose(bool bvolume,
				 bool bframe);
	/* Set current link lengths as resting lengths							*/
	void resetLinkRestLengths();
	/* Return the volume													*/
	Scalar getVolume() const;
	/* Cluster count														*/
	Vec3 getCenterOfMass() const
	{
		Vec3 com(0, 0, 0);
		for (i32 i = 0; i < m_nodes.size(); i++)
		{
			com += (m_nodes[i].m_x * this->getMass(i));
		}
		com /= this->getTotalMass();
		return com;
	}
	i32 clusterCount() const;
	/* Cluster center of mass												*/
	static Vec3 clusterCom(const Cluster* cluster);
	Vec3 clusterCom(i32 cluster) const;
	/* Cluster velocity at rpos												*/
	static Vec3 clusterVelocity(const Cluster* cluster, const Vec3& rpos);
	/* Cluster impulse														*/
	static void clusterVImpulse(Cluster* cluster, const Vec3& rpos, const Vec3& impulse);
	static void clusterDImpulse(Cluster* cluster, const Vec3& rpos, const Vec3& impulse);
	static void clusterImpulse(Cluster* cluster, const Vec3& rpos, const Impulse& impulse);
	static void clusterVAImpulse(Cluster* cluster, const Vec3& impulse);
	static void clusterDAImpulse(Cluster* cluster, const Vec3& impulse);
	static void clusterAImpulse(Cluster* cluster, const Impulse& impulse);
	static void clusterDCImpulse(Cluster* cluster, const Vec3& impulse);
	/* Generate bending constraints based on distance in the adjency graph	*/
	i32 generateBendingConstraints(i32 distance,
								   Material* mat = 0);
	/* Randomize constraints to reduce solver bias							*/
	void randomizeConstraints();

	void updateState(const AlignedObjectArray<Vec3>& qs, const AlignedObjectArray<Vec3>& vs);

	/* Release clusters														*/
	void releaseCluster(i32 index);
	void releaseClusters();
	/* Generate clusters (K-mean)											*/
	///generateClusters with k=0 will create a convex cluster for each tetrahedron or triangle
	///otherwise an approximation will be used (better performance)
	i32 generateClusters(i32 k, i32 maxiterations = 8192);
	/* Refine																*/
	void refine(ImplicitFn* ifn, Scalar accurary, bool cut);
	/* CutLink																*/
	bool cutLink(i32 node0, i32 node1, Scalar position);
	bool cutLink(const Node* node0, const Node* node1, Scalar position);

	///Ray casting using rayFrom and rayTo in worldspace, (not direction!)
	bool rayTest(const Vec3& rayFrom,
				 const Vec3& rayTo,
				 sRayCast& results);
	bool rayFaceTest(const Vec3& rayFrom,
					 const Vec3& rayTo,
					 sRayCast& results);
	i32 rayFaceTest(const Vec3& rayFrom, const Vec3& rayTo,
					Scalar& mint, i32& index) const;
	/* Solver presets														*/
	void setSolver(eSolverPresets::_ preset);
	/* predictMotion														*/
	void predictMotion(Scalar dt);
	/* solveConstraints														*/
	void solveConstraints();
	/* staticSolve															*/
	void staticSolve(i32 iterations);
	/* solveCommonConstraints												*/
	static void solveCommonConstraints(SoftBody** bodies, i32 count, i32 iterations);
	/* solveClusters														*/
	static void solveClusters(const AlignedObjectArray<SoftBody*>& bodies);
	/* integrateMotion														*/
	void integrateMotion();
	/* defaultCollisionHandlers												*/
	void defaultCollisionHandler(const CollisionObject2Wrapper* pcoWrap);
	void defaultCollisionHandler(SoftBody* psb);
	void setSelfCollision(bool useSelfCollision);
	bool useSelfCollision();
	void updateDeactivation(Scalar timeStep);
	void setZeroVelocity();
	bool wantsSleeping();

	virtual Matrix3x3 getImpulseFactor(i32 n_node)
	{
		Matrix3x3 tmp;
		tmp.setIdentity();
		return tmp;
	}

	//
	// Functionality to deal with new accelerated solvers.
	//

	/**
	 * Set a wind velocity for interaction with the air.
	 */
	void setWindVelocity(const Vec3& velocity);

	/**
	 * Return the wind velocity for interaction with the air.
	 */
	const Vec3& getWindVelocity();

	//
	// Set the solver that handles this soft body
	// Should not be allowed to get out of sync with reality
	// Currently called internally on addition to the world
	void setSoftBodySolver(SoftBodySolver* softBodySolver)
	{
		m_softBodySolver = softBodySolver;
	}

	//
	// Return the solver that handles this soft body
	//
	SoftBodySolver* getSoftBodySolver()
	{
		return m_softBodySolver;
	}

	//
	// Return the solver that handles this soft body
	//
	SoftBodySolver* getSoftBodySolver() const
	{
		return m_softBodySolver;
	}

	//
	// Cast
	//

	static const SoftBody* upcast(const CollisionObject2* colObj)
	{
		if (colObj->getInternalType() == CO_SOFT_BODY)
			return (const SoftBody*)colObj;
		return 0;
	}
	static SoftBody* upcast(CollisionObject2* colObj)
	{
		if (colObj->getInternalType() == CO_SOFT_BODY)
			return (SoftBody*)colObj;
		return 0;
	}

	//
	// ::CollisionObject2
	//

	virtual void getAabb(Vec3& aabbMin, Vec3& aabbMax) const
	{
		aabbMin = m_bounds[0];
		aabbMax = m_bounds[1];
	}
	//
	// Private
	//
	void pointersToIndices();
	void indicesToPointers(i32k* map = 0);

	i32 rayTest(const Vec3& rayFrom, const Vec3& rayTo,
				Scalar& mint, eFeature::_& feature, i32& index, bool bcountonly) const;
	void initializeFaceTree();
	void rebuildNodeTree();
	Vec3 evaluateCom() const;
	bool checkDeformableContact(const CollisionObject2Wrapper* colObjWrap, const Vec3& x, Scalar margin, SoftBody::sCti& cti, bool predict = false) const;
	bool checkDeformableFaceContact(const CollisionObject2Wrapper* colObjWrap, Face& f, Vec3& contact_point, Vec3& bary, Scalar margin, SoftBody::sCti& cti, bool predict = false) const;
	bool checkContact(const CollisionObject2Wrapper* colObjWrap, const Vec3& x, Scalar margin, SoftBody::sCti& cti) const;
	void updateNormals();
	void updateBounds();
	void updatePose();
	void updateConstants();
	void updateLinkConstants();
	void updateArea(bool averageArea = true);
	void initializeClusters();
	void updateClusters();
	void cleanupClusters();
	void prepareClusters(i32 iterations);
	void solveClusters(Scalar sor);
	void applyClusters(bool drift);
	void dampClusters();
	void setSpringStiffness(Scalar k);
	void setGravityFactor(Scalar gravFactor);
	void setCacheBarycenter(bool cacheBarycenter);
	void initializeDmInverse();
	void updateDeformation();
	void advanceDeformation();
	void applyForces();
	void setMaxStress(Scalar maxStress);
	void interpolateRenderMesh();
	void setCollisionQuadrature(i32 N);
	static void PSolve_Anchors(SoftBody* psb, Scalar kst, Scalar ti);
	static void PSolve_RContacts(SoftBody* psb, Scalar kst, Scalar ti);
	static void PSolve_SContacts(SoftBody* psb, Scalar, Scalar ti);
	static void PSolve_Links(SoftBody* psb, Scalar kst, Scalar ti);
	static void VSolve_Links(SoftBody* psb, Scalar kst);
	static psolver_t getSolver(ePSolver::_ solver);
	static vsolver_t getSolver(eVSolver::_ solver);
	void geometricCollisionHandler(SoftBody* psb);
#define SAFE_EPSILON SIMD_EPSILON * 100.0
	void updateNode(DbvtNode* node, bool use_velocity, bool margin)
	{
		if (node->isleaf())
		{
			SoftBody::Node* n = (SoftBody::Node*)(node->data);
			ATTRIBUTE_ALIGNED16(DbvtVolume)
			vol;
			Scalar pad = margin ? m_sst.radmrg : SAFE_EPSILON;  // use user defined margin or margin for floating point precision
			if (use_velocity)
			{
				Vec3 points[2] = {n->m_x, n->m_x + m_sst.sdt * n->m_v};
				vol = DbvtVolume::FromPoints(points, 2);
				vol.Expand(Vec3(pad, pad, pad));
			}
			else
			{
				vol = DbvtVolume::FromCR(n->m_x, pad);
			}
			node->volume = vol;
			return;
		}
		else
		{
			updateNode(node->childs[0], use_velocity, margin);
			updateNode(node->childs[1], use_velocity, margin);
			ATTRIBUTE_ALIGNED16(DbvtVolume)
			vol;
			Merge(node->childs[0]->volume, node->childs[1]->volume, vol);
			node->volume = vol;
		}
	}

	void updateNodeTree(bool use_velocity, bool margin)
	{
		if (m_ndbvt.m_root)
			updateNode(m_ndbvt.m_root, use_velocity, margin);
	}

	template <class DBVTNODE>  // DbvtNode or DbvntNode
	void updateFace(DBVTNODE* node, bool use_velocity, bool margin)
	{
		if (node->isleaf())
		{
			SoftBody::Face* f = (SoftBody::Face*)(node->data);
			Scalar pad = margin ? m_sst.radmrg : SAFE_EPSILON;  // use user defined margin or margin for floating point precision
			ATTRIBUTE_ALIGNED16(DbvtVolume)
			vol;
			if (use_velocity)
			{
				Vec3 points[6] = {f->m_n[0]->m_x, f->m_n[0]->m_x + m_sst.sdt * f->m_n[0]->m_v,
									   f->m_n[1]->m_x, f->m_n[1]->m_x + m_sst.sdt * f->m_n[1]->m_v,
									   f->m_n[2]->m_x, f->m_n[2]->m_x + m_sst.sdt * f->m_n[2]->m_v};
				vol = DbvtVolume::FromPoints(points, 6);
			}
			else
			{
				Vec3 points[3] = {f->m_n[0]->m_x,
									   f->m_n[1]->m_x,
									   f->m_n[2]->m_x};
				vol = DbvtVolume::FromPoints(points, 3);
			}
			vol.Expand(Vec3(pad, pad, pad));
			node->volume = vol;
			return;
		}
		else
		{
			updateFace(node->childs[0], use_velocity, margin);
			updateFace(node->childs[1], use_velocity, margin);
			ATTRIBUTE_ALIGNED16(DbvtVolume)
			vol;
			Merge(node->childs[0]->volume, node->childs[1]->volume, vol);
			node->volume = vol;
		}
	}
	void updateFaceTree(bool use_velocity, bool margin)
	{
		if (m_fdbvt.m_root)
			updateFace(m_fdbvt.m_root, use_velocity, margin);
		if (m_fdbvnt)
			updateFace(m_fdbvnt, use_velocity, margin);
	}

	template <typename T>
	static inline T BaryEval(const T& a,
							 const T& b,
							 const T& c,
							 const Vec3& coord)
	{
		return (a * coord.x() + b * coord.y() + c * coord.z());
	}

	void applyRepulsionForce(Scalar timeStep, bool applySpringForce)
	{
		AlignedObjectArray<i32> indices;
		{
			// randomize the order of repulsive force
			indices.resize(m_faceNodeContacts.size());
			for (i32 i = 0; i < m_faceNodeContacts.size(); ++i)
				indices[i] = i;
#define NEXTRAND (seed = (1664525L * seed + 1013904223L) & 0xffffffff)
			i32 i, ni;

			for (i = 0, ni = indices.size(); i < ni; ++i)
			{
				Swap(indices[i], indices[NEXTRAND % ni]);
			}
		}
		for (i32 k = 0; k < m_faceNodeContacts.size(); ++k)
		{
			i32 idx = indices[k];
			SoftBody::DeformableFaceNodeContact& c = m_faceNodeContacts[idx];
			SoftBody::Node* node = c.m_node;
			SoftBody::Face* face = c.m_face;
			const Vec3& w = c.m_bary;
			const Vec3& n = c.m_normal;
			Vec3 l = node->m_x - BaryEval(face->m_n[0]->m_x, face->m_n[1]->m_x, face->m_n[2]->m_x, w);
			Scalar d = c.m_margin - n.dot(l);
			d = d3Max(Scalar(0), d);

			const Vec3& va = node->m_v;
			Vec3 vb = BaryEval(face->m_n[0]->m_v, face->m_n[1]->m_v, face->m_n[2]->m_v, w);
			Vec3 vr = va - vb;
			const Scalar vn = Dot(vr, n);  // dn < 0 <==> opposing
			if (vn > OVERLAP_REDUCTION_FACTOR * d / timeStep)
				continue;
			Vec3 vt = vr - vn * n;
			Scalar I = 0;
			Scalar mass = node->m_im == 0 ? 0 : Scalar(1) / node->m_im;
			if (applySpringForce)
				I = -d3Min(m_repulsionStiffness * timeStep * d, mass * (OVERLAP_REDUCTION_FACTOR * d / timeStep - vn));
			if (vn < 0)
				I += 0.5 * mass * vn;
			i32 face_penetration = 0, node_penetration = node->m_constrained;
			for (i32 i = 0; i < 3; ++i)
				face_penetration |= face->m_n[i]->m_constrained;
			Scalar I_tilde = 2.0 * I / (1.0 + w.length2());

			//             double the impulse if node or face is constrained.
			if (face_penetration > 0 || node_penetration > 0)
			{
				I_tilde *= 2.0;
			}
			if (face_penetration <= 0)
			{
				for (i32 j = 0; j < 3; ++j)
					face->m_n[j]->m_v += w[j] * n * I_tilde * node->m_im;
			}
			if (node_penetration <= 0)
			{
				node->m_v -= I_tilde * node->m_im * n;
			}

			// apply frictional impulse
			Scalar vt_norm = vt.safeNorm();
			if (vt_norm > SIMD_EPSILON)
			{
				Scalar delta_vn = -2 * I * node->m_im;
				Scalar mu = c.m_friction;
				Scalar vt_new = d3Max(Scalar(1) - mu * delta_vn / (vt_norm + SIMD_EPSILON), Scalar(0)) * vt_norm;
				I = 0.5 * mass * (vt_norm - vt_new);
				vt.safeNormalize();
				I_tilde = 2.0 * I / (1.0 + w.length2());
				//                 double the impulse if node or face is constrained.
				if (face_penetration > 0 || node_penetration > 0)
					I_tilde *= 2.0;
				if (face_penetration <= 0)
				{
					for (i32 j = 0; j < 3; ++j)
						face->m_n[j]->m_v += w[j] * vt * I_tilde * (face->m_n[j])->m_im;
				}
				if (node_penetration <= 0)
				{
					node->m_v -= I_tilde * node->m_im * vt;
				}
			}
		}
	}
	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, class Serializer* serializer) const;
};

#endif  //_DRX3D_SOFT_BODY_H
