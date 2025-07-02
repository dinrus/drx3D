#include "ImportBspExample.h"

#include <drx3D/DynamicsCommon.h>

#include <drx3D/Maths/Linear/Quickprof.h"

#define QUAKE_BSP_IMPORTING 1

#ifdef QUAKE_BSP_IMPORTING
#include "BspLoader.h"
#include "BspConverter.h"
#endif  //QUAKE_BSP_IMPORTING

#include <stdio.h>  //printf debugging

#include <drx3D/Maths/Linear/AlignedObjectArray.h"

#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h"

///BspDemo shows the convex collision detection, by converting a Quake BSP file into convex objects and allowing interaction with boxes.
class BspDemo : public CommonRigidBodyBase
{
public:
	//keep the collision shapes, for deletion/cleanup

	BspDemo(struct GUIHelperInterface* helper)
		: CommonRigidBodyBase(helper)
	{
	}

	virtual ~BspDemo();

	virtual void initPhysics();

	void initPhysics(tukk bspfilename);

	virtual void resetCamera()
	{
		float dist = 43;
		float pitch = -12;
		float yaw = -175;
		float targetPos[3] = {4, -25, -6};
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}
};

#define CUBE_HALF_EXTENTS 1
#define EXTRA_HEIGHT -20.f

///BspToBulletConverter  extends the BspConverter to convert to drx3D datastructures
class BspToBulletConverter : public BspConverter
{
	BspDemo* m_demoApp;

public:
	BspToBulletConverter(BspDemo* demoApp)
		: m_demoApp(demoApp)
	{
	}

	virtual void addConvexVerticesCollider(AlignedObjectArray<Vec3>& vertices, bool isEntity, const Vec3& entityTargetLocation)
	{
		///perhaps we can do something special with entities (isEntity)
		///like adding a collision Triggering (as example)

		if (vertices.size() > 0)
		{
			float mass = 0.f;
			Transform2 startTransform;
			//can use a shift
			startTransform.setIdentity();
			startTransform.setOrigin(Vec3(0, 0, -10.f));
			//this create an internal copy of the vertices

			CollisionShape* shape = new ConvexHullShape(&(vertices[0].getX()), vertices.size());
			m_demoApp->m_collisionShapes.push_back(shape);

			//RigidBody* body = m_demoApp->localCreateRigidBody(mass, startTransform,shape);
			m_demoApp->createRigidBody(mass, startTransform, shape);
		}
	}
};

////////////////////////////////////

BspDemo::~BspDemo()
{
	exitPhysics();  //will delete all default data
}

void BspDemo::initPhysics()
{
	tukk bspfilename = "BspDemo.bsp";

	initPhysics(bspfilename);
}

void BspDemo::initPhysics(tukk bspfilename)
{
	i32 cameraUpAxis = 2;
	m_guiHelper->setUpAxis(cameraUpAxis);
	Vec3 grav(0, 0, 0);
	grav[cameraUpAxis] = -10;
	m_guiHelper->setUpAxis(cameraUpAxis);

	//_cameraUp = Vec3(0,0,1);
	//_forwardAxis = 1;

	//etCameraDistance(22.f);

	///Setup a Physics Simulation Environment

	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	//	CollisionShape* groundShape = new BoxShape(Vec3(50,3,50));
	m_dispatcher = new CollisionDispatcher(m_collisionConfiguration);
	Vec3 worldMin(-1000, -1000, -1000);
	Vec3 worldMax(1000, 1000, 1000);
	m_broadphase = new DbvtBroadphase();
	//m_broadphase = new AxisSweep3(worldMin,worldMax);
	//OverlappingPairCache* broadphase = new SimpleBroadphase();
	m_solver = new SequentialImpulseConstraintSolver();
	//ConstraintSolver* solver = new OdeConstraintSolver;
	m_dynamicsWorld = new DiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_dynamicsWorld->setGravity(grav);

#ifdef QUAKE_BSP_IMPORTING

	uk memoryBuffer = 0;

	tukk filename = "BspDemo.bsp";

	tukk prefix[] = {"./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
	i32 numPrefixes = sizeof(prefix) / sizeof(tukk);
	char relativeFileName[1024];
	FILE* file = 0;

	for (i32 i = 0; i < numPrefixes; i++)
	{
		sprintf(relativeFileName, "%s%s", prefix[i], filename);
		file = fopen(relativeFileName, "r");
		if (file)
			break;
	}

	if (file)
	{
		BspLoader bspLoader;
		i32 size = 0;
		if (fseek(file, 0, SEEK_END) || (size = ftell(file)) == EOF || fseek(file, 0, SEEK_SET))
		{ /* File operations denied? ok, just close and return failure */
			printf("Ошибка: cannot get filesize from %s\n", bspfilename);
		}
		else
		{
			//how to detect file size?
			memoryBuffer = malloc(size + 1);
			fread(memoryBuffer, 1, size, file);
			bspLoader.loadBSPFile(memoryBuffer);

			BspToBulletConverter bsp2bullet(this);
			float bspScaling = 0.1f;
			bsp2bullet.convertBsp(bspLoader, bspScaling);
		}
		fclose(file);
	}

#endif

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
}

//some code that de-mangles the windows filename passed in as argument
char cleaned_filename[512];
tuk getLastFileName()
{
	return cleaned_filename;
}
tuk makeExeToBspFilename(tukk lpCmdLine)
{
	// We might get a windows-style path on the command line, this can mess up the DOM which expects
	// all paths to be URI's.  This block of code does some conversion to try and make the input
	// compliant without breaking the ability to accept a properly formatted URI.  Right now this only
	// displays the first filename
	tukk in = lpCmdLine;
	tuk out = cleaned_filename;
	*out = '\0';
	// If the first character is a ", skip it (filenames with spaces in them are quoted)
	if (*in == '\"')
	{
		in++;
	}
	i32 i;
	for (i = 0; i < 512; i++)
	{
		//if we get '.' we stop as well, unless it's the first character. Then we add .bsp as extension
		// If we hit a null or a quote, stop copying.  This will get just the first filename.
		if (i && (in[0] == '.') && (in[1] == 'e') && (in[2] == 'x') && (in[3] == 'e'))
			break;

		// If we hit a null or a quote, stop copying.  This will get just the first filename.
		if (*in == '\0' || *in == '\"')
			break;
		// Copy while swapping backslashes for forward ones
		if (*in == '\\')
		{
			*out = '/';
		}
		else
		{
			*out = *in;
		}
		in++;
		out++;
	}
	*(out++) = '.';
	*(out++) = 'b';
	*(out++) = 's';
	*(out++) = 'p';
	*(out++) = 0;

	return cleaned_filename;
}

CommonExampleInterface* ImportBspCreateFunc(struct CommonExampleOptions& options)
{
	BspDemo* demo = new BspDemo(options.m_guiHelper);

	demo->initPhysics("BspDemo.bsp");
	return demo;
}
/*
static DemoApplication* Create()
	{
		BspDemo* demo = new BspDemo;
		demo->myinit();
		demo->initPhysics("BspDemo.bsp");
		return demo;
	}
	*/
