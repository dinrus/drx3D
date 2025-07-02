
#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/SoftBody/SoftRigidDynamicsWorld.h>

#include <drx3D/Physics/Collision/Dispatch/SphereSphereCollisionAlgorithm.h>
#include <drx3D/Physics/Collision/NarrowPhase/GjkEpa2.h>
#include <drx3D/Maths/Linear/Quickprof.h>
#include <drx3D/Maths/Linear/IDebugDraw.h>

#include "../BunnyMesh.h"
#include "../TorusMesh.h"
#include <stdio.h>  //printf debugging
#include <drx3D/Maths/Linear/ConvexHull.h>
#include <drx3D/Physics/SoftBody/SoftBodyRigidBodyCollisionConfiguration.h>
#include <drx3D/Physics/SoftBody/SoftBodyHelpers.h>

#include "../SoftDemo.h"
#include "../../GL_ShapeDrawer.h"

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <drx3D/Physics/SoftBody/SoftBody.h>

class BroadphaseInterface;
class CollisionShape;
class OverlappingPairCache;
class CollisionDispatcher;
class ConstraintSolver;
struct CollisionAlgorithmCreateFunc;
class DefaultCollisionConfiguration;

///collisions between two SoftBody's
class SoftSoftCollisionAlgorithm;

///collisions between a SoftBody and a RigidBody
class SoftRididCollisionAlgorithm;
class SoftRigidDynamicsWorld;

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>

class SoftDemo : public CommonRigidBodyBase
{
public:
	AlignedObjectArray<SoftSoftCollisionAlgorithm*> m_SoftSoftCollisionAlgorithms;

	AlignedObjectArray<SoftRididCollisionAlgorithm*> m_SoftRigidCollisionAlgorithms;

	SoftBodyWorldInfo m_softBodyWorldInfo;

	bool m_autocam;
	bool m_cutting;
	bool m_raycast;
	Scalar m_animtime;
	Clock m_clock;
	i32 m_lastmousepos[2];
	Vec3 m_impact;
	SoftBody::sRayCast m_results;
	SoftBody::Node* m_node;
	Vec3 m_goal;
	bool m_drag;

	//keep the collision shapes, for deletion/cleanup
	AlignedObjectArray<CollisionShape*> m_collisionShapes;

	BroadphaseInterface* m_broadphase;

	CollisionDispatcher* m_dispatcher;

	ConstraintSolver* m_solver;

	CollisionAlgorithmCreateFunc* m_boxBoxCF;

	DefaultCollisionConfiguration* m_collisionConfiguration;

public:
	void initPhysics();

	void exitPhysics();

	virtual void resetCamera()
	{
		//@todo depends on current_demo?
		float dist = 45;
		float pitch = -31;
		float yaw = 27;
		float targetPos[3] = {10 - 1, 0};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

	SoftDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper),
		  m_drag(false)

	{
	}
	virtual ~SoftDemo()
	{
		Assert(m_dynamicsWorld == 0);
	}

	//virtual void clientMoveAndDisplay();

	//virtual void displayCallback();

	void createStack(CollisionShape* boxShape, float halfCubeSize, i32 size, float zPos);

	virtual void setDrawClusters(bool drawClusters);

	virtual const SoftRigidDynamicsWorld* getSoftDynamicsWorld() const
	{
		///just make it a SoftRigidDynamicsWorld please
		///or we will add type checking
		return (SoftRigidDynamicsWorld*)m_dynamicsWorld;
	}

	virtual SoftRigidDynamicsWorld* getSoftDynamicsWorld()
	{
		///just make it a SoftRigidDynamicsWorld please
		///or we will add type checking
		return (SoftRigidDynamicsWorld*)m_dynamicsWorld;
	}

	//
	//void	clientResetScene();
	void renderme();
	void keyboardCallback(u8 key, i32 x, i32 y);
	void mouseFunc(i32 button, i32 state, i32 x, i32 y);
	void mouseMotionFunc(i32 x, i32 y);

	GUIHelperInterface* getGUIHelper()
	{
		return m_guiHelper;
	}

	virtual void renderScene()
	{
		CommonRigidBodyBase::renderScene();
		SoftRigidDynamicsWorld* softWorld = getSoftDynamicsWorld();

		for (i32 i = 0; i < softWorld->getSoftBodyArray().size(); i++)
		{
			SoftBody* psb = (SoftBody*)softWorld->getSoftBodyArray()[i];
			//if (softWorld->getDebugDrawer() && !(softWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
			{
				SoftBodyHelpers::DrawFrame(psb, softWorld->getDebugDrawer());
				SoftBodyHelpers::Draw(psb, softWorld->getDebugDrawer(), softWorld->getDrawFlags());
			}
		}
	}
};

#define MACRO_SOFT_DEMO(a)                    \
	class SoftDemo##a : public SoftDemo       \
	{                                         \
	public:                                   \
		static DemoApplication* Create()      \
		{                                     \
			SoftDemo* demo = new SoftDemo##a; \
			extern i32 current_demo;          \
			current_demo = a;                 \
			demo->initPhysics();              \
			return demo;                      \
		}                                     \
	};

//MACRO_SOFT_DEMO(0) //Init_Cloth
#if 0
MACRO_SOFT_DEMO(1) //Init_Pressure
MACRO_SOFT_DEMO(2)//Init_Volume
MACRO_SOFT_DEMO(3)//Init_Ropes
MACRO_SOFT_DEMO(4)//Init_Ropes_Attach
MACRO_SOFT_DEMO(5)//Init_ClothAttach
MACRO_SOFT_DEMO(6)//Init_Sticks
MACRO_SOFT_DEMO(7)//Init_Collide
MACRO_SOFT_DEMO(8)//Init_Collide2
MACRO_SOFT_DEMO(9)//Init_Collide3
MACRO_SOFT_DEMO(10)//Init_Impact
MACRO_SOFT_DEMO(11)//Init_Aero
MACRO_SOFT_DEMO(12)//Init_Friction
MACRO_SOFT_DEMO(13)//Init_Torus
MACRO_SOFT_DEMO(14)//Init_TorusMatch
MACRO_SOFT_DEMO(15)//Init_Bunny
MACRO_SOFT_DEMO(16)//Init_BunnyMatch
MACRO_SOFT_DEMO(17)//Init_Cutting1
MACRO_SOFT_DEMO(18)//Init_ClusterDeform
MACRO_SOFT_DEMO(19)//Init_ClusterCollide1
MACRO_SOFT_DEMO(20)//Init_ClusterCollide2
MACRO_SOFT_DEMO(21)//Init_ClusterSocket
MACRO_SOFT_DEMO(22)//Init_ClusterHinge
MACRO_SOFT_DEMO(23)//Init_ClusterCombine
MACRO_SOFT_DEMO(24)//Init_ClusterCar
MACRO_SOFT_DEMO(25)//Init_ClusterRobot
MACRO_SOFT_DEMO(26)//Init_ClusterStackSoft
MACRO_SOFT_DEMO(27)//Init_ClusterStackMixed
MACRO_SOFT_DEMO(28)//Init_TetraCube
MACRO_SOFT_DEMO(29)//Init_TetraBunny

#endif

extern float eye[3];
extern i32 glutScreenWidth;
extern i32 glutScreenHeight;

//static bool sDemoMode = false;

i32k maxProxies = 32766;
//i32k maxOverlap = 65535;

static Vec3* gGroundVertices = 0;
static i32* gGroundIndices = 0;
//static BvhTriangleMeshShape* trimeshShape =0;
//static RigidBody* staticBody = 0;
static float waveheight = 5.f;

const float TRIANGLE_SIZE = 8.f;
i32 current_demo = 20;
#define DEMO_MODE_TIMEOUT 15.f  //15 seconds for each demo

#ifdef _DEBUG
//i32k gNumObjects = 1;
#else
//i32k gNumObjects = 1;//try this in release mode: 3000. never go above 16384, unless you increate maxNumObjects  value in DemoApplication.cp
#endif

//i32k maxNumObjects = 32760;

#define CUBE_HALF_EXTENTS 1.5
#define EXTRA_HEIGHT -10.f

//
void SoftDemo::createStack(CollisionShape* boxShape, float halfCubeSize, i32 size, float zPos)
{
	Transform2 trans;
	trans.setIdentity();

	for (i32 i = 0; i < size; i++)
	{
		// This constructs a row, from left to right
		i32 rowSize = size - i;
		for (i32 j = 0; j < rowSize; j++)
		{
			Vec3 pos;
			pos.setVal(
				-rowSize * halfCubeSize + halfCubeSize + j * 2.0f * halfCubeSize,
				halfCubeSize + i * halfCubeSize * 2.0f,
				zPos);

			trans.setOrigin(pos);
			Scalar mass = 1.f;

			RigidBody* body = 0;
			body = createRigidBody(mass, trans, boxShape);
		}
	}
}

////////////////////////////////////
///for mouse picking
void pickingPreTickCallback(DynamicsWorld* world, Scalar timeStep)
{
	SoftDemo* softDemo = (SoftDemo*)world->getWorldUserInfo();

	if (softDemo->m_drag)
	{
		i32k x = softDemo->m_lastmousepos[0];
		i32k y = softDemo->m_lastmousepos[1];
		float rf[3];
		softDemo->getGUIHelper()->getRenderInterface()->getActiveCamera()->getCameraPosition(rf);
		float target[3];
		softDemo->getGUIHelper()->getRenderInterface()->getActiveCamera()->getCameraTargetPosition(target);
		Vec3 cameraTargetPosition(target[0], target[1], target[2]);

		const Vec3 cameraPosition(rf[0], rf[1], rf[2]);
		const Vec3 rayFrom = cameraPosition;

		const Vec3 rayTo = softDemo->getRayTo(x, y);
		const Vec3 rayDir = (rayTo - rayFrom).normalized();
		const Vec3 N = (cameraTargetPosition - cameraPosition).normalized();
		const Scalar O = Dot(softDemo->m_impact, N);
		const Scalar den = Dot(N, rayDir);
		if ((den * den) > 0)
		{
			const Scalar num = O - Dot(N, rayFrom);
			const Scalar hit = num / den;
			if ((hit > 0) && (hit < 1500))
			{
				softDemo->m_goal = rayFrom + rayDir * hit;
			}
		}
		Vec3 delta = softDemo->m_goal - softDemo->m_node->m_x;
		static const Scalar maxdrag = 10;
		if (delta.length2() > (maxdrag * maxdrag))
		{
			delta = delta.normalized() * maxdrag;
		}
		softDemo->m_node->m_v += delta / timeStep;
	}
}

//
// ImplicitShape
//

//
struct ImplicitSphere : SoftBody::ImplicitFn
{
	Vec3 center;
	Scalar sqradius;
	ImplicitSphere() {}
	ImplicitSphere(const Vec3& c, Scalar r) : center(c), sqradius(r * r) {}
	Scalar Eval(const Vec3& x)
	{
		return ((x - center).length2() - sqradius);
	}
};

//
// Tetra meshes
//

struct TetraBunny
{
#include "../bunny.inl"
};

struct TetraCube
{
#include "../cube.inl"
};

//
// Random
//

static inline Scalar UnitRand()
{
	return (rand() / (Scalar)RAND_MAX);
}

static inline Scalar SignedUnitRand()
{
	return (UnitRand() * 2 - 1);
}

static inline Vec3 Vector3Rand()
{
	const Vec3 p = Vec3(SignedUnitRand(), SignedUnitRand(), SignedUnitRand());
	return (p.normalized());
}

//
// Rb rain
//
static void Ctor_RbUpStack(SoftDemo* pdemo, i32 count)
{
	float mass = 10;

	CompoundShape* cylinderCompound = new CompoundShape;
	CollisionShape* cylinderShape = new CylinderShapeX(Vec3(4, 1, 1));
	CollisionShape* boxShape = new BoxShape(Vec3(4, 1, 1));
	Transform2 localTransform;
	localTransform.setIdentity();
	cylinderCompound->addChildShape(localTransform, boxShape);
	Quat orn(SIMD_HALF_PI, 0, 0);
	localTransform.setRotation(orn);
	//	localTransform.setOrigin(Vec3(1,1,1));
	cylinderCompound->addChildShape(localTransform, cylinderShape);

	CollisionShape* shape[] = {cylinderCompound,
								 new BoxShape(Vec3(1, 1, 1)),
								 new SphereShape(1.5)

	};
	static i32k nshapes = sizeof(shape) / sizeof(shape[0]);
	for (i32 i = 0; i < count; ++i)
	{
		Transform2 startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(Vec3(0, 2 + 6 * i, 0));
		pdemo->createRigidBody(mass, startTransform, shape[i % nshapes]);
		//pdemo->createRigidBody(mass,startTransform,shape[0]);
	}
}

//
// Big ball
//
static void Ctor_BigBall(SoftDemo* pdemo, Scalar mass = 10)
{
	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(0, 13, 0));
	pdemo->createRigidBody(mass, startTransform, new SphereShape(3));
}

//
// Big plate
//
static RigidBody* Ctor_BigPlate(SoftDemo* pdemo, Scalar mass = 15, Scalar height = 4)
{
	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(0, height, 0.5));
	RigidBody* body = pdemo->createRigidBody(mass, startTransform, new BoxShape(Vec3(5, 1, 5)));
	body->setFriction(1);
	return (body);
}

//
// Linear stair
//
static void Ctor_LinearStair(SoftDemo* pdemo, const Vec3& org, const Vec3& sizes, Scalar angle, i32 count)
{
	BoxShape* shape = new BoxShape(sizes);
	for (i32 i = 0; i < count; ++i)
	{
		Transform2 startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(org + Vec3(sizes.x() * i * 2, sizes.y() * i * 2, 0));
		RigidBody* body = pdemo->createRigidBody(0, startTransform, shape);
		body->setFriction(1);
	}
}

//
// Softbox
//
static SoftBody* Ctor_SoftBox(SoftDemo* pdemo, const Vec3& p, const Vec3& s)
{
	const Vec3 h = s * 0.5;
	const Vec3 c[] = {p + h * Vec3(-1, -1, -1),
						   p + h * Vec3(+1, -1, -1),
						   p + h * Vec3(-1, +1, -1),
						   p + h * Vec3(+1, +1, -1),
						   p + h * Vec3(-1, -1, +1),
						   p + h * Vec3(+1, -1, +1),
						   p + h * Vec3(-1, +1, +1),
						   p + h * Vec3(+1, +1, +1)};
	SoftBody* psb = SoftBodyHelpers::CreateFromConvexHull(pdemo->m_softBodyWorldInfo, c, 8);
	psb->generateBendingConstraints(2);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	return (psb);
}

//
// SoftBoulder
//
static SoftBody* Ctor_SoftBoulder(SoftDemo* pdemo, const Vec3& p, const Vec3& s, i32 np, i32 id)
{
	AlignedObjectArray<Vec3> pts;
	if (id) srand(id);
	for (i32 i = 0; i < np; ++i)
	{
		pts.push_back(Vector3Rand() * s + p);
	}
	SoftBody* psb = SoftBodyHelpers::CreateFromConvexHull(pdemo->m_softBodyWorldInfo, &pts[0], pts.size());
	psb->generateBendingConstraints(2);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	return (psb);
}

//#define TRACEDEMO { pdemo->demoname=__FUNCTION__+5;printf("Launching demo: " __FUNCTION__ "\r\n"); }

//
// Basic ropes
//
static void Init_Ropes(SoftDemo* pdemo)
{
	//TRACEDEMO
	i32k n = 15;
	for (i32 i = 0; i < n; ++i)
	{
		SoftBody* psb = SoftBodyHelpers::CreateRope(pdemo->m_softBodyWorldInfo, Vec3(-10, 0, i * 0.25),
														Vec3(10, 0, i * 0.25),
														16,
														1 + 2);
		psb->m_cfg.piterations = 4;
		psb->m_materials[0]->m_kLST = 0.1 + (i / (Scalar)(n - 1)) * 0.9;
		psb->setTotalMass(20);
		pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	}
}

//
// Rope attach
//
static void Init_RopeAttach(SoftDemo* pdemo)
{
	//TRACEDEMO
	pdemo->m_softBodyWorldInfo.m_sparsesdf.RemoveReferences(0);
	struct Functors
	{
		static SoftBody* CtorRope(SoftDemo* pdemo, const Vec3& p)
		{
			SoftBody* psb = SoftBodyHelpers::CreateRope(pdemo->m_softBodyWorldInfo, p, p + Vec3(10, 0, 0), 8, 1);
			psb->setTotalMass(50);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
			return (psb);
		}
	};
	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(12, 8, 0));
	RigidBody* body = pdemo->createRigidBody(50, startTransform, new BoxShape(Vec3(2, 6, 2)));
	SoftBody* psb0 = Functors::CtorRope(pdemo, Vec3(0, 8, -1));
	SoftBody* psb1 = Functors::CtorRope(pdemo, Vec3(0, 8, +1));
	psb0->appendAnchor(psb0->m_nodes.size() - 1, body);
	psb1->appendAnchor(psb1->m_nodes.size() - 1, body);
}

//
// Cloth attach
//
static void Init_ClothAttach(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar s = 4;
	const Scalar h = 6;
	i32k r = 9;
	SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, h, -s),
													 Vec3(+s, h, -s),
													 Vec3(-s, h, +s),
													 Vec3(+s, h, +s), r, r, 4 + 8, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(0, h, -(s + 3.5)));
	RigidBody* body = pdemo->createRigidBody(20, startTransform, new BoxShape(Vec3(s, 1, 3)));
	psb->appendAnchor(0, body);
	psb->appendAnchor(r - 1, body);
	pdemo->m_cutting = true;
}

//
// Impact
//
static void Init_Impact(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateRope(pdemo->m_softBodyWorldInfo, Vec3(0, 0, 0),
													Vec3(0, -1, 0),
													0,
													1);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	psb->m_cfg.kCHR = 0.5;
	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(0, 20, 0));
	pdemo->createRigidBody(10, startTransform, new BoxShape(Vec3(2, 2, 2)));
}

static void Init_CapsuleCollision(SoftDemo* pdemo)
{
#ifdef USE_AMD_OPENCL
	AlignedObjectArray<SoftBody*> emptyArray;
	if (g_openCLSIMDSolver)
		g_openCLSIMDSolver->optimize(emptyArray);
#endif  //USE_AMD_OPENCL

	//TRACEDEMO
	const Scalar s = 4;
	const Scalar h = 6;
	i32k r = 20;

	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(Vec3(0, h - 2, 0));

	CollisionShape* capsuleShape = new CapsuleShapeX(1, 5);
	capsuleShape->setMargin(0.5);

	//	capsule->setLocalScaling(Vec3(5,1,1));
	//	RigidBody*		body=pdemo->createRigidBody(20,startTransform,capsuleShape);
	RigidBody* body = pdemo->createRigidBody(0, startTransform, capsuleShape);
	body->setFriction(0.8f);

	i32 fixed = 0;  //4+8;
	SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, h, -s),
													 Vec3(+s, h, -s),
													 Vec3(-s, h, +s),
													 Vec3(+s, h, +s), r, r, fixed, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	psb->setTotalMass(0.1);

	psb->m_cfg.piterations = 10;
	psb->m_cfg.citerations = 10;
	psb->m_cfg.diterations = 10;
	//	psb->m_cfg.viterations = 10;

	//	psb->appendAnchor(0,body);
	//	psb->appendAnchor(r-1,body);
	//	pdemo->m_cutting=true;
}

//
// Collide
//
static void Init_Collide(SoftDemo* pdemo)
{
	//TRACEDEMO
	struct Functor
	{
		static SoftBody* Create(SoftDemo* pdemo, const Vec3& x, const Vec3& a)
		{
			SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVertices,
																   &gIndices[0][0],
																   NUM_TRIANGLES);
			psb->generateBendingConstraints(2);
			psb->m_cfg.piterations = 2;
			psb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
			psb->randomizeConstraints();
			Matrix3x3 m;
			m.setEulerZYX(a.x(), a.y(), a.z());
			psb->transform(Transform2(m, x));
			psb->scale(Vec3(2, 2, 2));
			psb->setTotalMass(50, true);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
			return (psb);
		}
	};
	for (i32 i = 0; i < 3; ++i)
	{
		Functor::Create(pdemo, Vec3(3 * i, 2, 0), Vec3(SIMD_PI / 2 * (1 - (i & 1)), SIMD_PI / 2 * (i & 1), 0));
	}
	pdemo->m_cutting = true;
}

//
// Collide2
//
static void Init_Collide2(SoftDemo* pdemo)
{
	//TRACEDEMO
	struct Functor
	{
		static SoftBody* Create(SoftDemo* pdemo, const Vec3& x, const Vec3& a)
		{
			SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVerticesBunny,
																   &gIndicesBunny[0][0],
																   BUNNY_NUM_TRIANGLES);
			SoftBody::Material* pm = psb->appendMaterial();
			pm->m_kLST = 0.5;
			pm->m_flags -= SoftBody::fMaterial::DebugDraw;
			psb->generateBendingConstraints(2, pm);
			psb->m_cfg.piterations = 2;
			psb->m_cfg.kDF = 0.5;
			psb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
			psb->randomizeConstraints();
			Matrix3x3 m;
			m.setEulerZYX(a.x(), a.y(), a.z());
			psb->transform(Transform2(m, x));
			psb->scale(Vec3(6, 6, 6));
			psb->setTotalMass(100, true);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
			return (psb);
		}
	};
	for (i32 i = 0; i < 3; ++i)
	{
		Functor::Create(pdemo, Vec3(0, -1 + 5 * i, 0), Vec3(0, SIMD_PI / 2 * (i & 1), 0));
	}
	pdemo->m_cutting = true;
}

//
// Collide3
//
static void Init_Collide3(SoftDemo* pdemo)
{
	//TRACEDEMO
	{
		const Scalar s = 8;
		SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, 0, -s),
														 Vec3(+s, 0, -s),
														 Vec3(-s, 0, +s),
														 Vec3(+s, 0, +s),
														 15, 15, 1 + 2 + 4 + 8, true);
		psb->m_materials[0]->m_kLST = 0.4;
		psb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
		psb->setTotalMass(150);
		pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	}
	{
		const Scalar s = 4;
		const Vec3 o = Vec3(5, 10, 0);
		SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo,
														 Vec3(-s, 0, -s) + o,
														 Vec3(+s, 0, -s) + o,
														 Vec3(-s, 0, +s) + o,
														 Vec3(+s, 0, +s) + o,
														 7, 7, 0, true);
		SoftBody::Material* pm = psb->appendMaterial();
		pm->m_kLST = 0.1;
		pm->m_flags -= SoftBody::fMaterial::DebugDraw;
		psb->generateBendingConstraints(2, pm);
		psb->m_materials[0]->m_kLST = 0.5;
		psb->m_cfg.collisions |= SoftBody::fCollision::VF_SS;
		psb->setTotalMass(150);
		pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
		pdemo->m_cutting = true;
	}
}

//
// Aerodynamic forces, 50x1g flyers
//
static void Init_Aero(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar s = 2;
	const Scalar h = 10;
	i32k segments = 6;
	i32k count = 50;
	for (i32 i = 0; i < count; ++i)
	{
		SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, h, -s),
														 Vec3(+s, h, -s),
														 Vec3(-s, h, +s),
														 Vec3(+s, h, +s),
														 segments, segments,
														 0, true);
		SoftBody::Material* pm = psb->appendMaterial();
		pm->m_flags -= SoftBody::fMaterial::DebugDraw;
		psb->generateBendingConstraints(2, pm);
		psb->m_cfg.kLF = 0.004;
		psb->m_cfg.kDG = 0.0003;
		psb->m_cfg.aeromodel = SoftBody::eAeroModel::V_TwoSided;
		Transform2 trs;
		Quat rot;
		Vec3 ra = Vector3Rand() * 0.1;
		Vec3 rp = Vector3Rand() * 15 + Vec3(0, 20, 80);
		rot.setEuler(SIMD_PI / 8 + ra.x(), -SIMD_PI / 7 + ra.y(), ra.z());
		trs.setIdentity();
		trs.setOrigin(rp);
		trs.setRotation(rot);
		psb->transform(trs);
		psb->setTotalMass(0.1);
		psb->addForce(Vec3(0, 2, 0), 0);
		pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	}
	pdemo->m_autocam = true;
}

static void Init_Aero2(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar s = 5;
	//psb->getWorldInfo()->m_gravity.setVal(0,0,0);

	i32k segments = 10;
	i32k count = 5;
	Vec3 pos(-s * segments, 0, 0);
	Scalar gap = 0.5;

	for (i32 i = 0; i < count; ++i)
	{
		SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, 0, -s * 3),
														 Vec3(+s, 0, -s * 3),
														 Vec3(-s, 0, +s),
														 Vec3(+s, 0, +s),
														 segments, segments * 3,
														 1 + 2, true);

		psb->getCollisionShape()->setMargin(0.5);
		SoftBody::Material* pm = psb->appendMaterial();
		pm->m_kLST = 0.0004;
		pm->m_flags -= SoftBody::fMaterial::DebugDraw;
		psb->generateBendingConstraints(2, pm);

		psb->m_cfg.kLF = 0.05;
		psb->m_cfg.kDG = 0.01;

		//psb->m_cfg.kLF			=	0.004;
		//psb->m_cfg.kDG			=	0.0003;

		psb->m_cfg.piterations = 2;
		psb->m_cfg.aeromodel = SoftBody::eAeroModel::V_TwoSidedLiftDrag;

		psb->setWindVelocity(Vec3(4, -12.0, -25.0));

		Transform2 trs;
		Quat rot;
		pos += Vec3(s * 2 + gap, 0, 0);
		rot.setRotation(Vec3(1, 0, 0), Scalar(SIMD_PI / 2));
		trs.setIdentity();
		trs.setOrigin(pos);
		trs.setRotation(rot);
		psb->transform(trs);
		psb->setTotalMass(2.0);

		//this could help performance in some cases
		SoftBodyHelpers::ReoptimizeLinkOrder(psb);

		pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	}

	pdemo->m_autocam = true;
}

//
// Friction
//
static void Init_Friction(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar bs = 2;
	const Scalar ts = bs + bs / 4;
	for (i32 i = 0, ni = 20; i < ni; ++i)
	{
		const Vec3 p(-ni * ts / 2 + i * ts, -10 + bs, 40);
		SoftBody* psb = Ctor_SoftBox(pdemo, p, Vec3(bs, bs, bs));
		psb->m_cfg.kDF = 0.1 * ((i + 1) / (Scalar)ni);
		psb->addVelocity(Vec3(0, 0, -10));
	}
}

//
// Pressure
//
static void Init_Pressure(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateEllipsoid(pdemo->m_softBodyWorldInfo, Vec3(35, 25, 0),
														 Vec3(1, 1, 1) * 3,
														 512);
	psb->m_materials[0]->m_kLST = 0.1;
	psb->m_cfg.kDF = 1;
	psb->m_cfg.kDP = 0.001;  // fun factor...
	psb->m_cfg.kPR = 2500;
	psb->setTotalMass(30, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	Ctor_BigPlate(pdemo);
	Ctor_LinearStair(pdemo, Vec3(0, 0, 0), Vec3(2, 1, 5), 0, 10);
	pdemo->m_autocam = true;
}

//
// Volume conservation
//
static void Init_Volume(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateEllipsoid(pdemo->m_softBodyWorldInfo, Vec3(35, 25, 0),
														 Vec3(1, 1, 1) * 3,
														 512);
	psb->m_materials[0]->m_kLST = 0.45;
	psb->m_cfg.kVC = 20;
	psb->setTotalMass(50, true);
	psb->setPose(true, false);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	Ctor_BigPlate(pdemo);
	Ctor_LinearStair(pdemo, Vec3(0, 0, 0), Vec3(2, 1, 5), 0, 10);
	pdemo->m_autocam = true;
}

//
// Stick+Bending+Rb's
//
static void Init_Sticks(SoftDemo* pdemo)
{
	//TRACEDEMO
	i32k n = 16;
	i32k sg = 4;
	const Scalar sz = 5;
	const Scalar hg = 4;
	const Scalar in = 1 / (Scalar)(n - 1);
	for (i32 y = 0; y < n; ++y)
	{
		for (i32 x = 0; x < n; ++x)
		{
			const Vec3 org(-sz + sz * 2 * x * in,
								-10,
								-sz + sz * 2 * y * in);
			SoftBody* psb = SoftBodyHelpers::CreateRope(pdemo->m_softBodyWorldInfo, org,
															org + Vec3(hg * 0.001, hg, 0),
															sg,
															1);
			psb->m_cfg.kDP = 0.005;
			psb->m_cfg.kCHR = 0.1;
			for (i32 i = 0; i < 3; ++i)
			{
				psb->generateBendingConstraints(2 + i);
			}
			psb->setMass(1, 0);
			psb->setTotalMass(0.01);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
		}
	}
	Ctor_BigBall(pdemo);
}

//
// Bending
//
static void Init_Bending(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar s = 4;
	const Vec3 x[] = {Vec3(-s, 0, -s),
						   Vec3(+s, 0, -s),
						   Vec3(+s, 0, +s),
						   Vec3(-s, 0, +s)};
	const Scalar m[] = {0, 0, 0, 1};
	SoftBody* psb = new SoftBody(&pdemo->m_softBodyWorldInfo, 4, x, m);
	psb->appendLink(0, 1);
	psb->appendLink(1, 2);
	psb->appendLink(2, 3);
	psb->appendLink(3, 0);
	psb->appendLink(0, 2);

	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
}

//
// 100kg cloth locked at corners, 10 falling 10kg rb's.
//
static void Init_Cloth(SoftDemo* pdemo)
{
	//TRACEDEMO
	const Scalar s = 8;
	SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, 0, -s),
													 Vec3(+s, 0, -s),
													 Vec3(-s, 0, +s),
													 Vec3(+s, 0, +s),
													 31, 31,
													 //		31,31,
													 1 + 2 + 4 + 8, true);

	psb->getCollisionShape()->setMargin(0.5);
	SoftBody::Material* pm = psb->appendMaterial();
	pm->m_kLST = 0.4;
	pm->m_flags -= SoftBody::fMaterial::DebugDraw;
	psb->generateBendingConstraints(2, pm);
	psb->setTotalMass(150);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	Ctor_RbUpStack(pdemo, 10);
	pdemo->m_cutting = true;
}

//
// 100kg Stanford's bunny
//
static void Init_Bunny(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVerticesBunny,
														   &gIndicesBunny[0][0],
														   BUNNY_NUM_TRIANGLES);
	SoftBody::Material* pm = psb->appendMaterial();
	pm->m_kLST = 0.5;
	pm->m_flags -= SoftBody::fMaterial::DebugDraw;
	psb->generateBendingConstraints(2, pm);
	psb->m_cfg.piterations = 2;
	psb->m_cfg.kDF = 0.5;
	psb->randomizeConstraints();
	psb->scale(Vec3(6, 6, 6));
	psb->setTotalMass(100, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	pdemo->m_cutting = true;
}

//
// 100kg Stanford's bunny with pose matching
//
static void Init_BunnyMatch(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVerticesBunny,
														   &gIndicesBunny[0][0],
														   BUNNY_NUM_TRIANGLES);
	psb->m_cfg.kDF = 0.5;
	psb->m_cfg.kMT = 0.05;
	psb->m_cfg.piterations = 5;
	psb->randomizeConstraints();
	psb->scale(Vec3(6, 6, 6));
	psb->setTotalMass(100, true);
	psb->setPose(false, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
}

//
// 50Kg Torus
//
static void Init_Torus(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVertices,
														   &gIndices[0][0],
														   NUM_TRIANGLES);
	psb->generateBendingConstraints(2);
	psb->m_cfg.piterations = 2;
	psb->randomizeConstraints();
	Matrix3x3 m;
	m.setEulerZYX(SIMD_PI / 2, 0, 0);
	psb->transform(Transform2(m, Vec3(0, 4, 0)));
	psb->scale(Vec3(2, 2, 2));
	psb->setTotalMass(50, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	pdemo->m_cutting = true;
}

//
// 50Kg Torus with pose matching
//
static void Init_TorusMatch(SoftDemo* pdemo)
{
	//TRACEDEMO
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVertices,
														   &gIndices[0][0],
														   NUM_TRIANGLES);
	psb->m_materials[0]->m_kLST = 0.1;
	psb->m_cfg.kMT = 0.05;
	psb->randomizeConstraints();
	Matrix3x3 m;
	m.setEulerZYX(SIMD_PI / 2, 0, 0);
	psb->transform(Transform2(m, Vec3(0, 4, 0)));
	psb->scale(Vec3(2, 2, 2));
	psb->setTotalMass(50, true);
	psb->setPose(false, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
}

//
// Cutting1
//
static void Init_Cutting1(SoftDemo* pdemo)
{
	const Scalar s = 6;
	const Scalar h = 2;
	i32k r = 16;
	const Vec3 p[] = {Vec3(+s, h, -s),
						   Vec3(-s, h, -s),
						   Vec3(+s, h, +s),
						   Vec3(-s, h, +s)};
	SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, p[0], p[1], p[2], p[3], r, r, 1 + 2 + 4 + 8, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	psb->m_cfg.piterations = 1;
	pdemo->m_cutting = true;
}

//
// Clusters
//

//
static void Ctor_Gear(SoftDemo* pdemo, const Vec3& pos, Scalar speed)
{
	Transform2 startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(pos);
	CompoundShape* shape = new CompoundShape();
#if 1
	shape->addChildShape(Transform2(Quat(0, 0, 0)), new BoxShape(Vec3(5, 1, 6)));
	shape->addChildShape(Transform2(Quat(0, 0, SIMD_HALF_PI)), new BoxShape(Vec3(5, 1, 6)));
#else
	shape->addChildShape(Transform2(Quat(0, 0, 0)), new CylinderShapeZ(Vec3(5, 1, 7)));
	shape->addChildShape(Transform2(Quat(0, 0, SIMD_HALF_PI)), new BoxShape(Vec3(4, 1, 8)));
#endif
	RigidBody* body = pdemo->createRigidBody(10, startTransform, shape);
	body->setFriction(1);
	DynamicsWorld* world = pdemo->getDynamicsWorld();
	HingeConstraint* hinge = new HingeConstraint(*body, Transform2::getIdentity());
	if (speed != 0) hinge->enableAngularMotor(true, speed, 3);
	world->addConstraint(hinge);
}

//
static SoftBody* Ctor_ClusterBunny(SoftDemo* pdemo, const Vec3& x, const Vec3& a)
{
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVerticesBunny, &gIndicesBunny[0][0], BUNNY_NUM_TRIANGLES);
	SoftBody::Material* pm = psb->appendMaterial();
	pm->m_kLST = 1;
	pm->m_flags -= SoftBody::fMaterial::DebugDraw;
	psb->generateBendingConstraints(2, pm);
	psb->m_cfg.piterations = 2;
	psb->m_cfg.kDF = 1;
	psb->m_cfg.collisions = SoftBody::fCollision::CL_SS +
							SoftBody::fCollision::CL_RS;
	psb->randomizeConstraints();
	Matrix3x3 m;
	m.setEulerZYX(a.x(), a.y(), a.z());
	psb->transform(Transform2(m, x));
	psb->scale(Vec3(8, 8, 8));
	psb->setTotalMass(150, true);
	psb->generateClusters(1);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	return (psb);
}

//
static SoftBody* Ctor_ClusterTorus(SoftDemo* pdemo, const Vec3& x, const Vec3& a, const Vec3& s = Vec3(2, 2, 2))
{
	SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVertices, &gIndices[0][0], NUM_TRIANGLES);
	SoftBody::Material* pm = psb->appendMaterial();
	pm->m_kLST = 1;
	pm->m_flags -= SoftBody::fMaterial::DebugDraw;
	psb->generateBendingConstraints(2, pm);
	psb->m_cfg.piterations = 2;
	psb->m_cfg.collisions = SoftBody::fCollision::CL_SS +
							SoftBody::fCollision::CL_RS;
	psb->randomizeConstraints();
	psb->scale(s);
	psb->rotate(Quat(a[0], a[1], a[2]));
	psb->translate(x);
	psb->setTotalMass(50, true);
	psb->generateClusters(64);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	return (psb);
}

//
static struct MotorControl : SoftBody::AJoint::IControl
{
	MotorControl()
	{
		goal = 0;
		maxtorque = 0;
	}
	Scalar Speed(SoftBody::AJoint*, Scalar current)
	{
		return (current + d3Min(maxtorque, d3Max(-maxtorque, goal - current)));
	}
	Scalar goal;
	Scalar maxtorque;
} motorcontrol;

//
struct SteerControl : SoftBody::AJoint::IControl
{
	SteerControl(Scalar s)
	{
		angle = 0;
		sign = s;
	}
	void Prepare(SoftBody::AJoint* joint)
	{
		joint->m_refs[0][0] = Cos(angle * sign);
		joint->m_refs[0][2] = Sin(angle * sign);
	}
	Scalar Speed(SoftBody::AJoint* joint, Scalar current)
	{
		return (motorcontrol.Speed(joint, current));
	}
	Scalar angle;
	Scalar sign;
};

static SteerControl steercontrol_f(+1);
static SteerControl steercontrol_r(-1);

//
static void Init_ClusterDeform(SoftDemo* pdemo)
{
	SoftBody* psb = Ctor_ClusterTorus(pdemo, Vec3(0, 0, 0), Vec3(SIMD_PI / 2, 0, SIMD_HALF_PI));
	psb->generateClusters(8);
	psb->m_cfg.kDF = 1;
}

//
static void Init_ClusterCollide1(SoftDemo* pdemo)
{
	const Scalar s = 8;
	SoftBody* psb = SoftBodyHelpers::CreatePatch(pdemo->m_softBodyWorldInfo, Vec3(-s, 0, -s),
													 Vec3(+s, 0, -s),
													 Vec3(-s, 0, +s),
													 Vec3(+s, 0, +s),
													 17, 17,  //9,9,//31,31,
													 1 + 2 + 4 + 8,
													 true);
	SoftBody::Material* pm = psb->appendMaterial();
	pm->m_kLST = 0.4;
	pm->m_flags -= SoftBody::fMaterial::DebugDraw;
	psb->m_cfg.kDF = 1;
	psb->m_cfg.kSRHR_CL = 1;
	psb->m_cfg.kSR_SPLT_CL = 0;
	psb->m_cfg.collisions = SoftBody::fCollision::CL_SS +

							SoftBody::fCollision::CL_RS;
	psb->generateBendingConstraints(2, pm);

	psb->getCollisionShape()->setMargin(0.05);
	psb->setTotalMass(50);

	///pass zero in generateClusters to create  cluster for each tetrahedron or triangle
	psb->generateClusters(0);
	//psb->generateClusters(64);

	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);

	Ctor_RbUpStack(pdemo, 10);
}

//
static void Init_ClusterCollide2(SoftDemo* pdemo)
{
	struct Functor
	{
		static SoftBody* Create(SoftDemo* pdemo, const Vec3& x, const Vec3& a)
		{
			SoftBody* psb = SoftBodyHelpers::CreateFromTriMesh(pdemo->m_softBodyWorldInfo, gVertices,
																   &gIndices[0][0],
																   NUM_TRIANGLES);
			SoftBody::Material* pm = psb->appendMaterial();
			pm->m_flags -= SoftBody::fMaterial::DebugDraw;
			psb->generateBendingConstraints(2, pm);
			psb->m_cfg.piterations = 2;
			psb->m_cfg.kDF = 1;
			psb->m_cfg.kSSHR_CL = 1;
			psb->m_cfg.kSS_SPLT_CL = 0;
			psb->m_cfg.kSKHR_CL = 0.1f;
			psb->m_cfg.kSK_SPLT_CL = 1;
			psb->m_cfg.collisions = SoftBody::fCollision::CL_SS +
									SoftBody::fCollision::CL_RS;
			psb->randomizeConstraints();
			Matrix3x3 m;
			m.setEulerZYX(a.x(), a.y(), a.z());
			psb->transform(Transform2(m, x));
			psb->scale(Vec3(2, 2, 2));
			psb->setTotalMass(50, true);
			psb->generateClusters(16);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
			return (psb);
		}
	};
	for (i32 i = 0; i < 3; ++i)
	{
		Functor::Create(pdemo, Vec3(3 * i, 2, 0), Vec3(SIMD_PI / 2 * (1 - (i & 1)), SIMD_PI / 2 * (i & 1), 0));
	}
}

//
static void Init_ClusterSocket(SoftDemo* pdemo)
{
	SoftBody* psb = Ctor_ClusterTorus(pdemo, Vec3(0, 0, 0), Vec3(SIMD_PI / 2, 0, SIMD_HALF_PI));
	RigidBody* prb = Ctor_BigPlate(pdemo, 50, 8);
	psb->m_cfg.kDF = 1;
	SoftBody::LJoint::Specs lj;
	lj.position = Vec3(0, 5, 0);
	psb->appendLinearJoint(lj, prb);
}

//
static void Init_ClusterHinge(SoftDemo* pdemo)
{
	SoftBody* psb = Ctor_ClusterTorus(pdemo, Vec3(0, 0, 0), Vec3(SIMD_PI / 2, 0, SIMD_HALF_PI));
	RigidBody* prb = Ctor_BigPlate(pdemo, 50, 8);
	psb->m_cfg.kDF = 1;
	SoftBody::AJoint::Specs aj;
	aj.axis = Vec3(0, 0, 1);
	psb->appendAngularJoint(aj, prb);
}

//
static void Init_ClusterCombine(SoftDemo* pdemo)
{
	const Vec3 sz(2, 4, 2);
	SoftBody* psb0 = Ctor_ClusterTorus(pdemo, Vec3(0, 8, 0), Vec3(SIMD_PI / 2, 0, SIMD_HALF_PI), sz);
	SoftBody* psb1 = Ctor_ClusterTorus(pdemo, Vec3(0, 8, 10), Vec3(SIMD_PI / 2, 0, SIMD_HALF_PI), sz);
	SoftBody* psbs[] = {psb0, psb1};
	for (i32 j = 0; j < 2; ++j)
	{
		psbs[j]->m_cfg.kDF = 1;
		psbs[j]->m_cfg.kDP = 0;
		psbs[j]->m_cfg.piterations = 1;
		psbs[j]->m_clusters[0]->m_matching = 0.05;
		psbs[j]->m_clusters[0]->m_ndamping = 0.05;
	}
	SoftBody::AJoint::Specs aj;
	aj.axis = Vec3(0, 0, 1);
	aj.icontrol = &motorcontrol;
	psb0->appendAngularJoint(aj, psb1);

	SoftBody::LJoint::Specs lj;
	lj.position = Vec3(0, 8, 5);
	psb0->appendLinearJoint(lj, psb1);
}

//
static void Init_ClusterCar(SoftDemo* pdemo)
{
	//	pdemo->setAzi(180);
	const Vec3 origin(100, 80, 0);
	const Quat orientation(-SIMD_PI / 2, 0, 0);
	const Scalar widthf = 8;
	const Scalar widthr = 9;
	const Scalar length = 8;
	const Scalar height = 4;
	const Vec3 wheels[] = {
		Vec3(+widthf, -height, +length),  // Front left
		Vec3(-widthf, -height, +length),  // Front right
		Vec3(+widthr, -height, -length),  // Rear left
		Vec3(-widthr, -height, -length),  // Rear right
	};
	SoftBody* pa = Ctor_ClusterBunny(pdemo, Vec3(0, 0, 0), Vec3(0, 0, 0));
	SoftBody* pfl = Ctor_ClusterTorus(pdemo, wheels[0], Vec3(0, 0, SIMD_HALF_PI), Vec3(2, 4, 2));
	SoftBody* pfr = Ctor_ClusterTorus(pdemo, wheels[1], Vec3(0, 0, SIMD_HALF_PI), Vec3(2, 4, 2));
	SoftBody* prl = Ctor_ClusterTorus(pdemo, wheels[2], Vec3(0, 0, SIMD_HALF_PI), Vec3(2, 5, 2));
	SoftBody* prr = Ctor_ClusterTorus(pdemo, wheels[3], Vec3(0, 0, SIMD_HALF_PI), Vec3(2, 5, 2));

	pfl->m_cfg.kDF =
		pfr->m_cfg.kDF =
			prl->m_cfg.kDF =
				prr->m_cfg.kDF = 1;

	SoftBody::LJoint::Specs lspecs;
	lspecs.cfm = 1;
	lspecs.erp = 1;
	lspecs.position = Vec3(0, 0, 0);

	lspecs.position = wheels[0];
	pa->appendLinearJoint(lspecs, pfl);
	lspecs.position = wheels[1];
	pa->appendLinearJoint(lspecs, pfr);
	lspecs.position = wheels[2];
	pa->appendLinearJoint(lspecs, prl);
	lspecs.position = wheels[3];
	pa->appendLinearJoint(lspecs, prr);

	SoftBody::AJoint::Specs aspecs;
	aspecs.cfm = 1;
	aspecs.erp = 1;
	aspecs.axis = Vec3(1, 0, 0);

	aspecs.icontrol = &steercontrol_f;
	pa->appendAngularJoint(aspecs, pfl);
	pa->appendAngularJoint(aspecs, pfr);

	aspecs.icontrol = &motorcontrol;
	pa->appendAngularJoint(aspecs, prl);
	pa->appendAngularJoint(aspecs, prr);

	pa->rotate(orientation);
	pfl->rotate(orientation);
	pfr->rotate(orientation);
	prl->rotate(orientation);
	prr->rotate(orientation);
	pa->translate(origin);
	pfl->translate(origin);
	pfr->translate(origin);
	prl->translate(origin);
	prr->translate(origin);
	pfl->m_cfg.piterations =
		pfr->m_cfg.piterations =
			prl->m_cfg.piterations =
				prr->m_cfg.piterations = 1;
	pfl->m_clusters[0]->m_matching =
		pfr->m_clusters[0]->m_matching =
			prl->m_clusters[0]->m_matching =
				prr->m_clusters[0]->m_matching = 0.05;
	pfl->m_clusters[0]->m_ndamping =
		pfr->m_clusters[0]->m_ndamping =
			prl->m_clusters[0]->m_ndamping =
				prr->m_clusters[0]->m_ndamping = 0.05;

	Ctor_LinearStair(pdemo, Vec3(0, -8, 0), Vec3(3, 2, 40), 0, 20);
	Ctor_RbUpStack(pdemo, 50);
	pdemo->m_autocam = true;
}

//
static void Init_ClusterRobot(SoftDemo* pdemo)
{
	struct Functor
	{
		static SoftBody* CreateBall(SoftDemo* pdemo, const Vec3& pos)
		{
			SoftBody* psb = SoftBodyHelpers::CreateEllipsoid(pdemo->m_softBodyWorldInfo, pos, Vec3(1, 1, 1) * 3, 512);
			psb->m_materials[0]->m_kLST = 0.45;
			psb->m_cfg.kVC = 20;
			psb->setTotalMass(50, true);
			psb->setPose(true, false);
			psb->generateClusters(1);
			pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
			return (psb);
		}
	};
	const Vec3 base = Vec3(0, 25, 8);
	SoftBody* psb0 = Functor::CreateBall(pdemo, base + Vec3(-8, 0, 0));
	SoftBody* psb1 = Functor::CreateBall(pdemo, base + Vec3(+8, 0, 0));
	SoftBody* psb2 = Functor::CreateBall(pdemo, base + Vec3(0, 0, +8 * Sqrt(2)));
	const Vec3 ctr = (psb0->clusterCom(0) + psb1->clusterCom(0) + psb2->clusterCom(0)) / 3;
	CylinderShape* pshp = new CylinderShape(Vec3(8, 1, 8));
	RigidBody* prb = pdemo->createRigidBody(50, Transform2(Quat(0, 0, 0), ctr + Vec3(0, 5, 0)), pshp);
	SoftBody::LJoint::Specs ls;
	ls.erp = 0.5f;
	ls.position = psb0->clusterCom(0);
	psb0->appendLinearJoint(ls, prb);
	ls.position = psb1->clusterCom(0);
	psb1->appendLinearJoint(ls, prb);
	ls.position = psb2->clusterCom(0);
	psb2->appendLinearJoint(ls, prb);

	BoxShape* pbox = new BoxShape(Vec3(20, 1, 40));
	RigidBody* pgrn;
	pgrn = pdemo->createRigidBody(0, Transform2(Quat(0, -SIMD_HALF_PI / 2, 0), Vec3(0, 0, 0)), pbox);

	pdemo->m_autocam = true;
}

//
static void Init_ClusterStackSoft(SoftDemo* pdemo)
{
	for (i32 i = 0; i < 10; ++i)
	{
		SoftBody* psb = Ctor_ClusterTorus(pdemo, Vec3(0, -9 + 8.25 * i, 0), Vec3(0, 0, 0));
		psb->m_cfg.kDF = 1;
	}
}

//
static void Init_ClusterStackMixed(SoftDemo* pdemo)
{
	for (i32 i = 0; i < 10; ++i)
	{
		if ((i + 1) & 1)
		{
			Ctor_BigPlate(pdemo, 50, -9 + 4.25 * i);
		}
		else
		{
			SoftBody* psb = Ctor_ClusterTorus(pdemo, Vec3(0, -9 + 4.25 * i, 0), Vec3(0, 0, 0));
			psb->m_cfg.kDF = 1;
		}
	}
}

//
// TetraBunny
//
static void Init_TetraBunny(SoftDemo* pdemo)
{
	SoftBody* psb = SoftBodyHelpers::CreateFromTetGenData(pdemo->m_softBodyWorldInfo,
															  TetraBunny::getElements(),
															  0,
															  TetraBunny::getNodes(),
															  false, true, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	psb->rotate(Quat(SIMD_PI / 2, 0, 0));
	psb->setVolumeMass(150);
	psb->m_cfg.piterations = 2;
	//psb->m_cfg.piterations=1;
	pdemo->m_cutting = false;
	//psb->getCollisionShape()->setMargin(0.01);
	psb->m_cfg.collisions = SoftBody::fCollision::CL_SS + SoftBody::fCollision::CL_RS
		//+ SoftBody::fCollision::CL_SELF
		;

	///pass zero in generateClusters to create  cluster for each tetrahedron or triangle
	psb->generateClusters(0);
	//psb->m_materials[0]->m_kLST=.2;
	psb->m_cfg.kDF = 10.;
}

//
// TetraCube
//
static void Init_TetraCube(SoftDemo* pdemo)
{
	SoftBody* psb = SoftBodyHelpers::CreateFromTetGenData(pdemo->m_softBodyWorldInfo,
															  TetraCube::getElements(),
															  0,
															  TetraCube::getNodes(),
															  false, true, true);
	pdemo->getSoftDynamicsWorld()->addSoftBody(psb);
	psb->scale(Vec3(4, 4, 4));
	psb->translate(Vec3(0, 5, 0));
	psb->setVolumeMass(300);

	///fix one vertex
	//psb->setMass(0,0);
	//psb->setMass(10,0);
	//psb->setMass(20,0);
	psb->m_cfg.piterations = 1;
	//psb->generateClusters(128);
	psb->generateClusters(16);
	//psb->getCollisionShape()->setMargin(0.5);

	psb->getCollisionShape()->setMargin(0.01);
	psb->m_cfg.collisions = SoftBody::fCollision::CL_SS + SoftBody::fCollision::CL_RS
		//+ SoftBody::fCollision::CL_SELF
		;
	psb->m_materials[0]->m_kLST = 0.8;
	pdemo->m_cutting = false;
}

/* Init		*/
void (*demofncs[])(SoftDemo*) =
	{
		Init_Cloth,
		Init_Pressure,
		Init_Volume,
		Init_Ropes,
		Init_RopeAttach,
		Init_ClothAttach,
		Init_Sticks,
		Init_CapsuleCollision,
		Init_Collide,
		Init_Collide2,
		Init_Collide3,
		Init_Impact,
		Init_Aero,
		Init_Aero2,
		Init_Friction,
		Init_Torus,
		Init_TorusMatch,
		Init_Bunny,
		Init_BunnyMatch,
		Init_Cutting1,
		Init_ClusterDeform,
		Init_ClusterCollide1,
		Init_ClusterCollide2,
		Init_ClusterSocket,
		Init_ClusterHinge,
		Init_ClusterCombine,
		Init_ClusterCar,
		Init_ClusterRobot,
		Init_ClusterStackSoft,
		Init_ClusterStackMixed,
		Init_TetraCube,
		Init_TetraBunny,
};

#if 0
void	SoftDemo::clientResetScene()
{
	m_azi = 0;
	m_cameraDistance = 30.f;
	m_cameraTargetPosition.setVal(0,0,0);


	/* Clean up	*/
	for(i32 i=m_dynamicsWorld->getNumCollisionObjects()-1;i>=0;i--)
	{
		CollisionObject2*	obj=m_dynamicsWorld->getCollisionObjectArray()[i];
		RigidBody*		body=RigidBody::upcast(obj);
		if(body&&body->getMotionState())
		{
			delete body->getMotionState();
		}
		while(m_dynamicsWorld->getNumConstraints())
		{
			btTypedConstraint*	pc=m_dynamicsWorld->getConstraint(0);
			m_dynamicsWorld->removeConstraint(pc);
			delete pc;
		}
		SoftBody* softBody = SoftBody::upcast(obj);
		if (softBody)
		{
			getSoftDynamicsWorld()->removeSoftBody(softBody);
		} else
		{
			RigidBody* body = RigidBody::upcast(obj);
			if (body)
				m_dynamicsWorld->removeRigidBody(body);
			else
				m_dynamicsWorld->removeCollisionObject(obj);
		}
		delete obj;
	}



}

#if 0
void SoftDemo::clientMoveAndDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);




	float ms = getDeltaTimeMicroseconds();
	float dt = ms / 1000000.f;//1.0/60.;



	if (m_dynamicsWorld)
	{

		if (sDemoMode)
		{
			static float demoCounter = DEMO_MODE_TIMEOUT;
			demoCounter-= dt;
			if (demoCounter<0)
			{

				demoCounter=DEMO_MODE_TIMEOUT;
				current_demo++;
				current_demo=current_demo%(sizeof(demofncs)/sizeof(demofncs[0]));
				clientResetScene();
			}
		}


//#define FIXED_STEP
#ifdef FIXED_STEP
		m_dynamicsWorld->stepSimulation(dt=1.0f/60.f,0);

#else
		//during idle mode, just run 1 simulation step maximum, otherwise 4 at max
	//	i32 maxSimSubSteps = m_idle ? 1 : 4;
		//if (m_idle)
		//	dt = 1.0/420.f;

		i32 numSimSteps;
		numSimSteps = m_dynamicsWorld->stepSimulation(dt);
		//numSimSteps = m_dynamicsWorld->stepSimulation(dt,10,1./240.f);

#ifdef VERBOSE_TIMESTEPPING_CONSOLEOUTPUT
		if (!numSimSteps)
			printf("Interpolated transforms\n");
		else
		{
			if (numSimSteps > maxSimSubSteps)
			{
				//detect dropping frames
				printf("Dropped (%i) simulation steps out of %i\n",numSimSteps - maxSimSubSteps,numSimSteps);
			} else
			{
				printf("Simulated (%i) steps\n",numSimSteps);
			}
		}
#endif  //VERBOSE_TIMESTEPPING_CONSOLEOUTPUT

#endif

#ifdef USE_AMD_OPENCL
		if (g_openCLSIMDSolver)
			g_openCLSIMDSolver->copyBackToSoftBodies();
#endif  //USE_AMD_OPENCL

		if(m_drag)
		{
			m_node->m_v*=0;
		}

		m_softBodyWorldInfo.m_sparsesdf.GarbageCollect();

		//optional but useful: debug drawing

	}

#ifdef USE_QUICKPROF
	Profiler::beginBlock("render");
#endif  //USE_QUICKPROF

	renderme();

	//render the graphics objects, with center of mass shift

	updateCamera();

#ifdef USE_QUICKPROF
	Profiler::endBlock("render");
#endif
	glFlush();

	swapBuffers();

}
#endif

#if 0
void	SoftDemo::renderme()
{
	btIDebugDraw*	idraw=m_dynamicsWorld->getDebugDrawer();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	m_dynamicsWorld->debugDrawWorld();

	//i32 debugMode = m_dynamicsWorld->getDebugDrawer()? m_dynamicsWorld->getDebugDrawer()->getDebugMode() : -1;

	SoftRigidDynamicsWorld* softWorld = (SoftRigidDynamicsWorld*)m_dynamicsWorld;
	//IDebugDraw*	sdraw = softWorld ->getDebugDrawer();


	for (  i32 i=0;i<softWorld->getSoftBodyArray().size();i++)
	{
		SoftBody*	psb=(SoftBody*)softWorld->getSoftBodyArray()[i];
		if (softWorld->getDebugDrawer() && !(softWorld->getDebugDrawer()->getDebugMode() & (IDebugDraw::DBG_DrawWireframe)))
		{
			SoftBodyHelpers::DrawFrame(psb,softWorld->getDebugDrawer());
			SoftBodyHelpers::Draw(psb,softWorld->getDebugDrawer(),softWorld->getDrawFlags());
		}
	}

	/* Bodies		*/
	Vec3	ps(0,0,0);
	i32			nps=0;

	SoftBodyArray&	sbs=getSoftDynamicsWorld()->getSoftBodyArray();
	for(i32 ib=0;ib<sbs.size();++ib)
	{
		SoftBody*	psb=sbs[ib];
		nps+=psb->m_nodes.size();
		for(i32 i=0;i<psb->m_nodes.size();++i)
		{
			ps+=psb->m_nodes[i].m_x;
		}
	}
	ps/=nps;
	if(m_autocam)
		m_cameraTargetPosition+=(ps-m_cameraTargetPosition)*0.05;
	/* Anm			*/
	if(!isIdle())
		m_animtime=m_clock.getTimeMilliseconds()/1000.f;
	/* Ray cast		*/
	if(m_raycast)
	{
		/* Prepare rays	*/
		i32k		res=64;
		const Scalar	fres=res-1;
		const Scalar	size=8;
		const Scalar	dist=10;
		Transform2		trs;
		trs.setOrigin(ps);
		Scalar rayLength = 1000.f;

		const Scalar	angle=m_animtime*0.2;
		trs.setRotation(Quat(angle,SIMD_PI/4,0));
		Vec3	dir=trs.getBasis()*Vec3(0,-1,0);
		trs.setOrigin(ps-dir*dist);
		AlignedObjectArray<Vec3>	origins;
		AlignedObjectArray<Scalar>	fractions;
		origins.resize(res*res);
		fractions.resize(res*res,1.f);
		for(i32 y=0;y<res;++y)
		{
			for(i32 x=0;x<res;++x)
			{
				i32k	idx=y*res+x;
				origins[idx]=trs*Vec3(-size+size*2*x/fres,dist,-size+size*2*y/fres);
			}
		}
		/* Cast rays	*/
		{
			m_clock.reset();
			if (sbs.size())
			{
				Vec3*		org=&origins[0];
				Scalar*				fraction=&fractions[0];
				SoftBody**			psbs=&sbs[0];
				SoftBody::sRayCast	results;
				for(i32 i=0,ni=origins.size(),nb=sbs.size();i<ni;++i)
				{
					for(i32 ib=0;ib<nb;++ib)
					{
						Vec3 rayFrom = *org;
						Vec3 rayTo = rayFrom+dir*rayLength;
						if(psbs[ib]->rayTest(rayFrom,rayTo,results))
						{
							*fraction=results.fraction;
						}
					}
					++org;++fraction;
				}
				long	ms=d3Max<long>(m_clock.getTimeMilliseconds(),1);
				long	rayperseconds=(1000*(origins.size()*sbs.size()))/ms;
				printf("%d ms (%d rays/s)\r\n",i32(ms),i32(rayperseconds));
			}
		}
		/* Draw rays	*/
		const Vec3	c[]={	origins[0],
			origins[res-1],
			origins[res*(res-1)],
			origins[res*(res-1)+res-1]};
		idraw->drawLine(c[0],c[1],Vec3(0,0,0));
		idraw->drawLine(c[1],c[3],Vec3(0,0,0));
		idraw->drawLine(c[3],c[2],Vec3(0,0,0));
		idraw->drawLine(c[2],c[0],Vec3(0,0,0));
		for(i32 i=0,ni=origins.size();i<ni;++i)
		{
			const Scalar		fraction=fractions[i];
			const Vec3&	org=origins[i];
			if(fraction<1.f)
			{
				idraw->drawLine(org,org+dir*rayLength*fraction,Vec3(1,0,0));
			}
			else
			{
				idraw->drawLine(org,org-dir*rayLength*0.1,Vec3(0,0,0));
			}
		}
#undef RES
	}
	/* Water level	*/
	static const Vec3	axis[]={Vec3(1,0,0),
		Vec3(0,1,0),
		Vec3(0,0,1)};
	if(m_softBodyWorldInfo.water_density>0)
	{
		const Vec3	c=	Vec3((Scalar)0.25,(Scalar)0.25,1);
		const Scalar	a=	(Scalar)0.5;
		const Vec3	n=	m_softBodyWorldInfo.water_normal;
		const Vec3	o=	-n*m_softBodyWorldInfo.water_offset;
		const Vec3	x=	btCross(n,axis[n.minAxis()]).normalized();
		const Vec3	y=	btCross(x,n).normalized();
		const Scalar	s=	25;
		idraw->drawTriangle(o-x*s-y*s,o+x*s-y*s,o+x*s+y*s,c,a);
		idraw->drawTriangle(o-x*s-y*s,o+x*s+y*s,o-x*s+y*s,c,a);
	}
	//

	i32 lineWidth=280;
	i32 xStart = m_glutScreenWidth - lineWidth;
	i32 yStart = 20;

	if((getDebugMode() & btIDebugDraw::DBG_NoHelpText)==0)
	{
		setOrthographicProjection();
		glDisable(GL_LIGHTING);
		glColor3f(0, 0, 0);
		char buf[124];

		glRasterPos3f(xStart, yStart, 0);
		if (sDemoMode)
		{
			sprintf(buf,"d to toggle demo mode (on)");
		} else
		{
			sprintf(buf,"d to toggle demo mode (off)");
		}
		GLDebugDrawString(xStart,20,buf);
		glRasterPos3f(xStart, yStart, 0);
		sprintf(buf,"] for next demo (%d)",current_demo);
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);
		glRasterPos3f(xStart, yStart, 0);
		sprintf(buf,"c to visualize clusters");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);
		glRasterPos3f(xStart, yStart, 0);
		sprintf(buf,"; to toggle camera mode");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);
		glRasterPos3f(xStart, yStart, 0);
        sprintf(buf,"n,m,l,k for power and steering");
		yStart+=20;
		GLDebugDrawString(xStart,yStart,buf);


		resetPerspectiveProjection();
		glEnable(GL_LIGHTING);
	}

	DemoApplication::renderme();

}
#endif
#endif

void SoftDemo::setDrawClusters(bool drawClusters)
{
	if (drawClusters)
	{
		getSoftDynamicsWorld()->setDrawFlags(getSoftDynamicsWorld()->getDrawFlags() | fDrawFlags::Clusters);
	}
	else
	{
		getSoftDynamicsWorld()->setDrawFlags(getSoftDynamicsWorld()->getDrawFlags() & (~fDrawFlags::Clusters));
	}
}

#if 0
void	SoftDemo::keyboardCallback(u8 key, i32 x, i32 y)
{
	switch(key)
	{
	case    'd':	sDemoMode = !sDemoMode; break;
	case	'n':	motorcontrol.maxtorque=10;motorcontrol.goal+=1;break;
	case	'm':	motorcontrol.maxtorque=10;motorcontrol.goal-=1;break;
	case	'l':	steercontrol_f.angle+=0.1;steercontrol_r.angle+=0.1;break;
	case	'k':	steercontrol_f.angle-=0.1;steercontrol_r.angle-=0.1;break;
	case	']':	++current_demo;clientResetScene();break;
	case	'[':	--current_demo;clientResetScene();break;
	case	',':	m_raycast=!m_raycast;break;
	case	';':	m_autocam=!m_autocam;break;
	case	'c':	getSoftDynamicsWorld()->setDrawFlags(getSoftDynamicsWorld()->getDrawFlags()^fDrawFlags::Clusters);break;
	case	'`':
		{
			SoftBodyArray&	sbs=getSoftDynamicsWorld()->getSoftBodyArray();
			for(i32 ib=0;ib<sbs.size();++ib)
			{
				SoftBody*	psb=sbs[ib];
				psb->staticSolve(128);
			}
		}
		break;
	default:		DemoApplication::keyboardCallback(key,x,y);
	}
}
#endif
//
void SoftDemo::mouseMotionFunc(i32 x, i32 y)
{
	if (m_node && (m_results.fraction < 1.f))
	{
		if (!m_drag)
		{
#define SQ(_x_) (_x_) * (_x_)
			if ((SQ(x - m_lastmousepos[0]) + SQ(y - m_lastmousepos[1])) > 6)
			{
				m_drag = true;
			}
#undef SQ
		}
		if (m_drag)
		{
			m_lastmousepos[0] = x;
			m_lastmousepos[1] = y;
		}
	}
}

#if 0
//
void	SoftDemo::mouseFunc(i32 button, i32 state, i32 x, i32 y)
{
	if(button==0)
	{
		switch(state)
		{
			case	0:
			{
				m_results.fraction=1.f;
				DemoApplication::mouseFunc(button,state,x,y);
				if(!m_pickConstraint)
				{
					const Vec3			rayFrom=m_cameraPosition;
					const Vec3			rayTo=getRayTo(x,y);
					const Vec3			rayDir=(rayTo-rayFrom).normalized();
					SoftBodyArray&		sbs=getSoftDynamicsWorld()->getSoftBodyArray();
					for(i32 ib=0;ib<sbs.size();++ib)
					{
						SoftBody*				psb=sbs[ib];
						SoftBody::sRayCast	res;
						if(psb->rayTest(rayFrom,rayTo,res))
						{
							m_results=res;
						}
					}
					if(m_results.fraction<1.f)
					{
						m_impact			=	rayFrom+(rayTo-rayFrom)*m_results.fraction;
						m_drag				=	m_cutting ? false : true;
						m_lastmousepos[0]	=	x;
						m_lastmousepos[1]	=	y;
						m_node				=	0;
						switch(m_results.feature)
						{
						case SoftBody::eFeature::Tetra:
							{
								SoftBody::Tetra&	tet=m_results.body->m_tetras[m_results.index];
								m_node=tet.m_n[0];
								for(i32 i=1;i<4;++i)
								{
									if(	(m_node->m_x-m_impact).length2()>
										(tet.m_n[i]->m_x-m_impact).length2())
									{
										m_node=tet.m_n[i];
									}
								}
								break;
							}
						case	SoftBody::eFeature::Face:
							{
								SoftBody::Face&	f=m_results.body->m_faces[m_results.index];
								m_node=f.m_n[0];
								for(i32 i=1;i<3;++i)
								{
									if(	(m_node->m_x-m_impact).length2()>
										(f.m_n[i]->m_x-m_impact).length2())
									{
										m_node=f.m_n[i];
									}
								}
							}
							break;
						}
						if(m_node) m_goal=m_node->m_x;
						return;
					}
				}
			}
			break;
		case	1:
			if((!m_drag)&&m_cutting&&(m_results.fraction<1.f))
			{
				ImplicitSphere	isphere(m_impact,1);
				printf("Mass before: %f\r\n",m_results.body->getTotalMass());
				m_results.body->refine(&isphere,0.0001,true);
				printf("Mass after: %f\r\n",m_results.body->getTotalMass());
			}
			m_results.fraction=1.f;
			m_drag=false;
			DemoApplication::mouseFunc(button,state,x,y);
			break;
		}
	}
	else
	{
		DemoApplication::mouseFunc(button,state,x,y);
	}
}
#endif

void SoftDemo::initPhysics()
{
	///create concave ground mesh

	m_guiHelper->setUpAxis(1);
	//	m_azi = 0;

	//reset and disable motorcontrol at the start
	motorcontrol.goal = 0;
	motorcontrol.maxtorque = 0;

	CollisionShape* groundShape = 0;
	{
		i32 i;
		i32 j;

		i32k NUM_VERTS_X = 30;
		i32k NUM_VERTS_Y = 30;
		i32k totalVerts = NUM_VERTS_X * NUM_VERTS_Y;
		i32k totalTriangles = 2 * (NUM_VERTS_X - 1) * (NUM_VERTS_Y - 1);

		gGroundVertices = new Vec3[totalVerts];
		gGroundIndices = new i32[totalTriangles * 3];

		Scalar offset(-50);

		for (i = 0; i < NUM_VERTS_X; i++)
		{
			for (j = 0; j < NUM_VERTS_Y; j++)
			{
				gGroundVertices[i + j * NUM_VERTS_X].setVal((i - NUM_VERTS_X * 0.5f) * TRIANGLE_SIZE,
															  //0.f,
															  waveheight * sinf((float)i) * cosf((float)j + offset),
															  (j - NUM_VERTS_Y * 0.5f) * TRIANGLE_SIZE);
			}
		}

		i32 vertStride = sizeof(Vec3);
		i32 indexStride = 3 * sizeof(i32);

		i32 index = 0;
		for (i = 0; i < NUM_VERTS_X - 1; i++)
		{
			for (i32 j = 0; j < NUM_VERTS_Y - 1; j++)
			{
				gGroundIndices[index++] = j * NUM_VERTS_X + i;
				gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
				gGroundIndices[index++] = j * NUM_VERTS_X + i + 1;
				;

				gGroundIndices[index++] = j * NUM_VERTS_X + i;
				gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i;
				gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
			}
		}

		TriangleIndexVertexArray* indexVertexArrays = new TriangleIndexVertexArray(totalTriangles,
																					   gGroundIndices,
																					   indexStride,
																					   totalVerts, (Scalar*)&gGroundVertices[0].x(), vertStride);

		bool useQuantizedAabbCompression = true;

		groundShape = new BvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);
		groundShape->setMargin(0.5);
	}

	m_collisionShapes.push_back(groundShape);

	CollisionShape* groundBox = new BoxShape(Vec3(100, CUBE_HALF_EXTENTS, 100));
	m_collisionShapes.push_back(groundBox);

	CompoundShape* cylinderCompound = new CompoundShape;
	CollisionShape* cylinderShape = new CylinderShape(Vec3(CUBE_HALF_EXTENTS, CUBE_HALF_EXTENTS, CUBE_HALF_EXTENTS));
	Transform2 localTransform;
	localTransform.setIdentity();
	cylinderCompound->addChildShape(localTransform, cylinderShape);
	Quat orn(Vec3(0, 1, 0), SIMD_PI);
	localTransform.setRotation(orn);
	cylinderCompound->addChildShape(localTransform, cylinderShape);

	m_collisionShapes.push_back(cylinderCompound);

	m_dispatcher = 0;

	///register some softbody collision algorithms on top of the default btDefaultCollisionConfiguration
	m_collisionConfiguration = new SoftBodyRigidBodyCollisionConfiguration();

	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
	m_softBodyWorldInfo.m_dispatcher = m_dispatcher;

	////////////////////////////
	///Register softbody versus softbody collision algorithm

	///Register softbody versus rigidbody collision algorithm

	////////////////////////////

	Vec3 worldAabbMin(-1000, -1000, -1000);
	Vec3 worldAabbMax(1000, 1000, 1000);

	m_broadphase = new AxisSweep3(worldAabbMin, worldAabbMax, maxProxies);

	m_softBodyWorldInfo.m_broadphase = m_broadphase;

	SequentialImpulseConstraintSolver* solver = new SequentialImpulseConstraintSolver();

	m_solver = solver;

	SoftBodySolver* softBodySolver = 0;
#ifdef USE_AMD_OPENCL

	static bool once = true;
	if (once)
	{
		once = false;
		initCL(0, 0);
	}

	if (g_openCLSIMDSolver)
		delete g_openCLSIMDSolver;
	if (g_softBodyOutput)
		delete g_softBodyOutput;

	if (1)
	{
		g_openCLSIMDSolver = new OpenCLSoftBodySolverSIMDAware(g_cqCommandQue, g_cxMainContext);
		//	g_openCLSIMDSolver = new OpenCLSoftBodySolver( g_cqCommandQue, g_cxMainContext);
		g_openCLSIMDSolver->setCLFunctions(new CachingCLFunctions(g_cqCommandQue, g_cxMainContext));
	}

	softBodySolver = g_openCLSIMDSolver;
	g_softBodyOutput = new SoftBodySolverOutputCLtoCPU;
#endif  //USE_AMD_OPENCL

	DiscreteDynamicsWorld* world = new SoftRigidDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration, softBodySolver);
	m_dynamicsWorld = world;
	m_dynamicsWorld->setInternalTickCallback(pickingPreTickCallback, this, true);

	m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
	m_dynamicsWorld->setGravity(Vec3(0, -10, 0));
	m_softBodyWorldInfo.m_gravity.setVal(0, -10, 0);
	m_guiHelper->createPhysicsDebugDrawer(world);
	//	clientResetScene();

	m_softBodyWorldInfo.m_sparsesdf.Initialize();
	//	clientResetScene();

	//create ground object
	Transform2 tr;
	tr.setIdentity();
	tr.setOrigin(Vec3(0, -12, 0));

	CollisionObject2* newOb = new CollisionObject2();
	newOb->setWorldTransform(tr);
	newOb->setInterpolationWorldTransform(tr);
	i32 lastDemo = (sizeof(demofncs) / sizeof(demofncs[0])) - 1;

	if (current_demo < 0)
		current_demo = lastDemo;
	if (current_demo > lastDemo)
		current_demo = 0;

	if (current_demo > 19)
	{
		newOb->setCollisionShape(m_collisionShapes[0]);
	}
	else
	{
		newOb->setCollisionShape(m_collisionShapes[1]);
	}

	m_dynamicsWorld->addCollisionObject(newOb);

	m_softBodyWorldInfo.m_sparsesdf.Reset();

	motorcontrol.goal = 0;
	motorcontrol.maxtorque = 0;

	m_softBodyWorldInfo.air_density = (Scalar)1.2;
	m_softBodyWorldInfo.water_density = 0;
	m_softBodyWorldInfo.water_offset = 0;
	m_softBodyWorldInfo.water_normal = Vec3(0, 0, 0);
	m_softBodyWorldInfo.m_gravity.setVal(0, -10, 0);

	m_autocam = false;
	m_raycast = false;
	m_cutting = false;
	m_results.fraction = 1.f;

	demofncs[current_demo](this);

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

void SoftDemo::exitPhysics()
{
	//cleanup in the reverse order of creation/initialization

	//remove the rigidbodies from the dynamics world and delete them
	i32 i;
	for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		RigidBody* body = RigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (i32 j = 0; j < m_collisionShapes.size(); j++)
	{
		CollisionShape* shape = m_collisionShapes[j];
		m_collisionShapes[j] = 0;
		delete shape;
	}

	//delete dynamics world
	delete m_dynamicsWorld;
	m_dynamicsWorld = 0;

	//delete solver
	delete m_solver;

	//delete broadphase
	delete m_broadphase;

	//delete dispatcher
	delete m_dispatcher;

	delete m_collisionConfiguration;
}

class CommonExampleInterface* SoftDemoCreateFunc(struct CommonExampleOptions& options)
{
	current_demo = options.m_option;
	return new SoftDemo(options.m_guiHelper);
}
