// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//////////////////////////////////////////////////////////////////////////
#include <drx3D/PhysX/StdAfx.h>
//////////////////////////////////////////////////////////////////////////

// for visual debugger connection, use at least profile or debug, release does not work
#define USE_PHYSX_DEBUGDRAW      (true)
#define USE_PHYSX_DEBUGDRAWCONTACTPOINTS (false)
#define USE_PHYSX_MOVE_ROOT (false)

// comment out to use release libs of PhysX, define to use PROFILE libs
#define USE_PHYSX_PROFILE
#define USE_PHYSX_VISUALDEBUGGER
//#define _DEBUG

//////////////////////////////////////////////////////////////////////////

#define PHYSX_TIMESTEP (1.0f/50.0f)

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// physX release -> in drxengine release
// physX profile -> in drxengine release
// physX debug   -> in drxengine debug
#ifdef _DEBUG
#ifdef USE_PHYSX_PROFILE
#undef USE_PHYSX_PROFILE
#endif
#endif

#if defined(USE_PHYSX_VISUALDEBUGGER) && !defined(USE_PHYSX_PROFILE)
#define USE_PHYSX_PROFILE
#endif

// NOTE: when compiling libs from PhysX 3.4 sources,
// switch PhysXExtensions, PhysXVehicle, and PhysXCharacterKinematic 
// to Multi-theread DLL runtime library (in C++/Code Generation)

#if defined(_DEBUG)
#pragma comment(lib, "PhysX3DEBUG_x64.lib")
#pragma comment(lib, "PhysX3CommonDEBUG_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib, "PxFoundationDEBUG_x64.lib")
#pragma comment(lib, "PxPvdSDKDEBUG_x64.lib")
#pragma comment(lib, "PhysX3CookingDEBUG_x64.lib")
#pragma comment(lib, "PhysX3VehicleDEBUG.lib")
#else
#if defined(USE_PHYSX_PROFILE)
#pragma comment(lib, "PhysX3PROFILE_x64.lib")
#pragma comment(lib, "PhysX3CommonPROFILE_x64.lib")
#pragma comment(lib, "PhysX3ExtensionsPROFILE.lib")
#pragma comment(lib, "PxFoundationPROFILE_x64.lib")
#pragma comment(lib, "PxPvdSDKPROFILE_x64.lib")
#pragma comment(lib, "PhysX3CookingPROFILE_x64.lib")
#pragma comment(lib, "PhysX3VehiclePROFILE.lib")
#else
#pragma comment(lib, "PhysX3_x64.lib")
#pragma comment(lib, "PhysX3Common_x64.lib")
#pragma comment(lib, "PhysX3Extensions.lib")
#pragma comment(lib, "PxFoundation_x64.lib")
#pragma comment(lib, "PxPvdSDK_x64.lib")
#pragma comment(lib, "PhysX3Cooking_x64.lib")
#pragma comment(lib, "PhysX3Vehicle.lib")
#endif
#endif

//////////////////////////////////////////////////////////////////////////

#include <drx3D/PhysX/DrxPhysX.h>

//////////////////////////////////////////////////////////////////////////

#include <drx3D/PhysX/PxPhysicsAPI.h>
#include <drx3D/PhysX/extensions/PxDefaultErrorCallback.h>
#include <drx3D/PhysX/extensions/PxDefaultAllocator.h>
#include <drx3D/PhysX/foundation/PxMat33.h>
#include <drx3D/PhysX/foundation/PxMat44.h>
#include <drx3D/PhysX/pvd/PxPvd.h>

using namespace physx;

//////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace cpx // DrxPhysX helper
{
	namespace Helper
	{
		void Log(STxt s) {
			//std::cerr << s << std::endl;
			DrxLogAlways("%s", s.c_str());
		}
	}

	namespace CPX_Error
	{
		static void fatalError(STxt s)
		{
			Helper::Log("DrxPhysX.cpp - Fatal Error: " + s);
			exit(0);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	DrxPhysX::DrxPhysX() :
		m_dt(PHYSX_TIMESTEP),
		m_Scene(nullptr),
		m_Cooking(nullptr),
		m_Foundation(nullptr),
		m_Physics(nullptr),
		m_CpuDispatcher(nullptr),
		m_PvdTransport(nullptr),
 		m_Pvd(nullptr),
		m_DebugVisualizationForAllSceneElements(false)
	{
	}

	PxFilterFlags CollFilter(PxFilterObjectAttributes attributes0, PxFilterData fd0, 
													 PxFilterObjectAttributes attributes1,	PxFilterData fd1,
							             PxPairFlags& pairFlags,
							             ukk constantBlock, PxU32 constantBlockSize)
	{
		// word0 - flags
		// word1 - flagsCollider
		// word2 - entity id (-1 = terrain)
		// word3 - bits 0..3 - simclass; 4 - enable CCD; 5 - ignore terrain; 6 - if set word2 is id-to-always-collide-with
		if ((!(fd0.word0 & fd1.word1 & -(i32)fd1.word3>>31 | fd1.word0 & fd0.word1 & -(i32)fd0.word3>>31) || max(fd0.word3&7,fd1.word3&7)>3)
			  && !((fd0.word3 | fd1.word3) & 64 && fd0.word2==fd1.word2)
			  || (fd1.word2 & -((i32)fd0.word3>>5))==-1)
			return PxFilterFlag::eKILL;
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS | PxPairFlag::ePRE_SOLVER_VELOCITY;
		//if ((fd0.word2 | fd1.word2) & 16)
			pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |= PxPairFlag::eCONTACT_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	void DrxPhysX::Init()
	{
		if (m_Physics)
			return;
		using namespace physx;
		using namespace CPX_Error;

		static bool isInitialized = false;
		if (isInitialized) return;
		isInitialized = true;

		// create foundation
		m_Foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_DefaultAllocatorCallback, m_DefaultErrorCallback);
		if (!m_Foundation)
		{
			fatalError("PxCreateFoundation failed!");
		}

		// create cooking
		m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, PxCookingParams(PxTolerancesScale()));
		if (!m_Cooking)
		{
			fatalError("PxCreateCooking failed!");
		}

		// create physics context
		bool recordMemoryAllocations = false;
		m_Pvd = PxCreatePvd(*m_Foundation);
		if (!m_Pvd)
		{
			fatalError("PxCreatePvd failed!");
		}

		m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), recordMemoryAllocations, m_Pvd);
		if (!m_Physics)
		{
			fatalError("PxCreatePhysics failed!");
		}

		if (!PxInitExtensions(*m_Physics, m_Pvd))
		{
			fatalError("PxInitExtensions failed!");
		}

		// finished
		Helper::Log("PhysX - initialized.\n");

		PxRegisterUnifiedHeightFields(*m_Physics);

		PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());

		// PhysX Visual Debugger - connection
 #if defined(USE_PHYSX_VISUALDEBUGGER)
		{
			// setup connection parameters
			tukk pvd_host_ip = "localhost";  // IP of the PC which is running PVD
			i32 port = 5425;                        // TCP port to connect to, where PVD is listening
 													//u32 timeout = 100;          // timeout in milliseconds to wait for PVD to respond,
			u32 timeout = 10000;           // timeout in milliseconds to wait for PVD to respond,
 													// consoles and remote PCs need a higher timeout.
			m_PvdTransport = PxDefaultPvdSocketTransportCreate(pvd_host_ip, port, timeout);
			if (!m_PvdTransport) fatalError("PxDefaultPvdSocketTransportCreate failed!");

			// and now try to connect
			m_Pvd->connect(*m_PvdTransport, PxPvdInstrumentationFlag::eALL);
			if (m_Pvd->isConnected()) Helper::Log("PhysX Visual Debugger Connection - initialized.\n");
		}
#endif

		i32k noOfThreads = 3; // 1
		const PxVec3 gravity = PxVec3(0.0f, 0.0f, -9.81f);

		sceneDesc.gravity = gravity;
		m_CpuDispatcher = PxDefaultCpuDispatcherCreate(noOfThreads);
		if (!m_CpuDispatcher) fatalError("PxDefaultCpuDispatcherCreate failed!");
		sceneDesc.cpuDispatcher = m_CpuDispatcher;
		sceneDesc.filterShader = CollFilter;
		sceneDesc.broadPhaseType = PxBroadPhaseType::eMBP;
		sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
		sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
		//sceneDesc.flags |= PxSceneFlag::eADAPTIVE_FORCE | PxSceneFlag::eENABLE_AVERAGE_POINT;

		m_Scene = m_Physics->createScene(sceneDesc);
		if (!m_Scene) fatalError("createScene failed!");

		m_Scene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);

		if (USE_PHYSX_DEBUGDRAW)
		{
			m_Scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 0.0f);
			m_Scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			m_Scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 0.0f);
			m_Scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 0.0f);

			m_Scene->setVisualizationParameter(PxVisualizationParameter::eCONTACT_POINT, 1.0f);
			m_Scene->setVisualizationParameter(PxVisualizationParameter::eBODY_ANG_VELOCITY, 0.0f);
		}

		// move root for nvidia physx visual debugger...
		/*if (USE_PHYSX_MOVE_ROOT)
		{
			physx::PxVec3 shift(0, 0, 32.0f);
			m_Scene->shiftOrigin(shift);
		} /* */

		PxInitVehicleSDK(*m_Physics);
		PxVehicleSetBasisVectors(PxVec3(0,0,1),PxVec3(0,1,0));
		PxVehicleSetUpdateMode(PxVehicleUpdateMode::eVELOCITY_CHANGE);
	}

	DrxPhysX::~DrxPhysX()
	{
		// shutdown PhysX
		PxCloseVehicleSDK();
		m_Cooking->release();
		m_Scene->release();
		m_CpuDispatcher->release();
		m_Physics->release();
		m_Pvd->release();
		if (m_PvdTransport) m_PvdTransport->release();
		PxCloseExtensions();
		m_Foundation->release();
	}

	void DrxPhysX::SetDebugVisualizationForAllSceneElements(bool enable)
	{
		if (enable == m_DebugVisualizationForAllSceneElements) return;
		m_DebugVisualizationForAllSceneElements = enable;

		std::vector<PxActor*> vActors;
		physx::PxActorTypeFlags flags = physx::PxActorTypeFlag::eRIGID_STATIC | physx::PxActorTypeFlag::eRIGID_DYNAMIC;
		vActors.resize(m_Scene->getNbActors( flags ));
		i32 nActors = m_Scene->getActors(flags, &vActors[0], vActors.size());

		for (i32 actorIdx = 0; actorIdx < vActors.size(); actorIdx++)
		{
			physx::PxRigidActor* actor = nullptr;
			if (!(actor = vActors[actorIdx]->isRigidActor())) continue;
			actor->setActorFlag(physx::PxActorFlag::eVISUALIZATION, enable);
	
			std::vector<PxShape*> vShapes;
			vShapes.resize(actor->getNbShapes());
			i32 nShapes = actor->getShapes(vShapes.data(), vShapes.size());

			// Iterate over each shape
			for (i32 shapeIdx = 0; shapeIdx < vShapes.size(); shapeIdx++)
			{
				PxShape* shape = vShapes[shapeIdx];
				if (shape->getGeometryType() == physx::PxGeometryType::eHEIGHTFIELD) shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, false);
				else shape->setFlag(physx::PxShapeFlag::eVISUALIZATION, enable);
			}
		}
	}

	inline void _SceneResetEntities(physx::PxScene* scene, physx::PxActorTypeFlags flags)
	{
		if (!scene) return;

		PxU32 nActors = scene->getNbActors(flags);
		PxActor** actors = new PxActor*[nActors];
		scene->getActors(flags, actors, nActors);
		scene->removeActors(actors, nActors);
		delete[] actors;
	}

	void DrxPhysX::SceneClear() // removes all entities from scene
	{
		if (!m_Scene) return;
		_SceneResetEntities(m_Scene, PxActorTypeFlag::eRIGID_STATIC | PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::ePARTICLE_SYSTEM | PxActorTypeFlag::ePARTICLE_FLUID | PxActorTypeFlag::eCLOTH);
	}

	void DrxPhysX::SceneResetDynamicEntities() // removes dynamic entities from scene
	{
		if (!m_Scene) return;
		_SceneResetEntities(m_Scene, PxActorTypeFlag::eRIGID_DYNAMIC);
	}

	void DrxPhysX::DisconnectPhysicsDebugger()
	{
		if (m_Pvd->isConnected()) m_Pvd->disconnect();
	}
}

//////////////////////////////////////////////////////////////////////////

namespace cpx {
	DrxPhysX g_drxPhysX; // PhysX context
}
