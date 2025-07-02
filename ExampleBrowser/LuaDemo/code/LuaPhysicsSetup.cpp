#include "../LuaPhysicsSetup.h"

#include <drx3D/Common/Interfaces/CommonMultiBodyBase.h>
#include <drx3D/Importers/URDF/UrdfImporter.h>
#include <drx3D/Importers/URDF/MyMultiBodyCreator.h>
#include <drx3D/Importers/URDF/URDF2Bullet.h>

struct LuaPhysicsSetup : public CommonMultiBodyBase
{
	LuaPhysicsSetup(GUIHelperInterface* helper);
	virtual ~LuaPhysicsSetup();

	virtual void initPhysics();

	virtual void exitPhysics();

	virtual void stepSimulation(float deltaTime)
	{
		m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
		CommonMultiBodyBase::stepSimulation(deltaTime);
	}
};

#include <drx3D/DynamicsCommon.h>
#include <drx3D/Maths/Linear/Vec3.h>
#include <iostream>

extern "C"
{
#include <X/lua/lua.h>
#include <X/lua/lualib.h>
#include <X/lua/lauxlib.h>
}

tukk sLuaFileName = "init_physics.lua";
static i32 upaxis = 1;

//tukk sLuaFileName = "init_urdf.lua";
//static i32 upaxis = 2;

static const float scaling = 0.35f;
static LuaPhysicsSetup* sLuaDemo = 0;

static Vec4 colors[4] =
	{
		Vec4(1, 0, 0, 1),
		Vec4(0, 1, 0, 1),
		Vec4(0, 1, 1, 1),
		Vec4(1, 1, 0, 1),
};

LuaPhysicsSetup::LuaPhysicsSetup(GUIHelperInterface* helper)
	: CommonMultiBodyBase(helper)
{
	sLuaDemo = this;
}

LuaPhysicsSetup::~LuaPhysicsSetup()
{
	sLuaDemo = 0;
}

//todo: allow to create solver, broadphase, multiple worlds etc.
static i32 gCreateDefaultDynamicsWorld(lua_State* L)
{
	sLuaDemo->createEmptyDynamicsWorld();
	Vec3 grav(0, 0, 0);
	grav[upaxis] = -10;

	sLuaDemo->m_dynamicsWorld->setGravity(grav);
	sLuaDemo->m_guiHelper->createPhysicsDebugDrawer(sLuaDemo->m_dynamicsWorld);
	lua_pushlightuserdata(L, sLuaDemo->m_dynamicsWorld);
	return 1;
}

static i32 gDeleteDynamicsWorld(lua_State* L)
{
	return 0;
}

ATTRIBUTE_ALIGNED16(struct)
CustomRigidBodyData
{
	i32 m_graphicsInstanceIndex;
};

static i32 gCreateCubeShape(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 4)
	{
		Vec3 halfExtents(1, 1, 1);
		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument to createCubeShape should be world";
			return 0;
		}
		//expect userdata = sLuaDemo->m_dynamicsWorld
		halfExtents = Vec3(lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
		CollisionShape* colShape = new BoxShape(halfExtents);

		lua_pushlightuserdata(L, colShape);
		return 1;
	}
	else
	{
		std::cerr << "Ошибка: неверное число аргументов для createCubeShape, ожидалось 4 (world,halfExtentsX,halfExtentsY,halfExtentsX). но получено " << argc;
	}
	return 0;
}

static i32 gCreateSphereShape(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 2)
	{
		Vec3 halfExtents(1, 1, 1);
		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument to createSphereShape should be world";
			return 0;
		}
		//expect userdata = sLuaDemo->m_dynamicsWorld
		Scalar radius = lua_tonumber(L, 2);
		CollisionShape* colShape = new SphereShape(radius);

		lua_pushlightuserdata(L, colShape);
		return 1;
	}
	else
	{
		std::cerr << "Ошибка: неверное число аргументов для createSphereShape, ожидалось 2 (world,radius), но получено " << argc;
	}
	return 0;
}

i32 luaL_returnlen(lua_State* L, i32 index)
{
	lua_len(L, index);
	i32 len = lua_tointeger(L, -1);
	lua_pop(L, 1);
	return len;
}

Vec3 getLuaVectorArg(lua_State* L, i32 index)
{
	Vec3 pos(0, 0, 0);

	i32 sz = luaL_returnlen(L, index);  // get size of table
	{
		lua_rawgeti(L, index, 1);  // push t[i]
		pos[0] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);  // push t[i]
		pos[1] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);  // push t[i]
		pos[2] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	return pos;
}

Quat getLuaQuatArg(lua_State* L, i32 index)
{
	Quat orn(0, 0, 0, 1);

	i32 sz = luaL_returnlen(L, index);  // get size of table
	{
		lua_rawgeti(L, index, 1);  // push t[i]
		orn[0] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 2);  // push t[i]
		orn[1] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 3);  // push t[i]
		orn[2] = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_rawgeti(L, index, 4);  // push t[i]
		orn[3] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	return orn;
}

static i32 gLoadMultiBodyFromUrdf(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 4)
	{
		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument to b3CreateRigidbody should be world";
			return 0;
		}

		luaL_checktype(L, 3, LUA_TTABLE);

		Vec3 pos = getLuaVectorArg(L, 3);

		Quat orn = getLuaQuatArg(L, 4);

		DiscreteDynamicsWorld* world = (DiscreteDynamicsWorld*)lua_touserdata(L, 1);
		if (world != sLuaDemo->m_dynamicsWorld)
		{
			std::cerr << "ошибка: first argument expected to be a world";
			return 0;
		}
		tukk fileName = lua_tostring(L, 2);
#if 1
		URDFImporter u2b(sLuaDemo->m_guiHelper, 0);
		bool loadOk = u2b.loadURDF(fileName);
		if (loadOk)
		{
			drx3DPrintf("loaded %s OK!", fileName);

			Transform2 tr;
			tr.setIdentity();
			tr.setOrigin(pos);
			tr.setRotation(orn);
			i32 rootLinkIndex = u2b.getRootLinkIndex();
			//			printf("urdf root link index = %d\n",rootLinkIndex);
			MyMultiBodyCreator creation(sLuaDemo->m_guiHelper);
			bool m_useMultiBody = true;
			ConvertURDF2Bullet(u2b, creation, tr, sLuaDemo->m_dynamicsWorld, m_useMultiBody, u2b.getPathPrefix());
			MultiBody* mb = creation.getBulletMultiBody();

			if (mb)
			{
				lua_pushlightuserdata(L, mb);
				return 1;
			}
		}
		else
		{
			drx3DPrintf("can't find %s", fileName);
		}
#endif
	}

	return 0;
}

static i32 gCreateRigidBody(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 5)
	{
		Transform2 startTransform;
		startTransform.setIdentity();

		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument to b3CreateRigidbody should be world";
			return 0;
		}
		DiscreteDynamicsWorld* world = (DiscreteDynamicsWorld*)lua_touserdata(L, 1);
		if (world != sLuaDemo->m_dynamicsWorld)
		{
			std::cerr << "ошибка: first argument expected to be a world";
			return 0;
		}

		if (!lua_isuserdata(L, 2))
		{
			std::cerr << "ошибка: second argument to b3CreateRigidbody should be collision shape";
			return 0;
		}

		Scalar mass = lua_tonumber(L, 3);

		luaL_checktype(L, 4, LUA_TTABLE);

		Vec3 pos = getLuaVectorArg(L, 4);

		Quat orn = getLuaQuatArg(L, 5);

		CollisionShape* colShape = (CollisionShape*)lua_touserdata(L, 2);
		//expect userdata = sLuaDemo->m_dynamicsWorld

		Vec3 inertia(0, 0, 0);
		if (mass)
		{
			colShape->calculateLocalInertia(mass, inertia);
		}

		RigidBody* body = new RigidBody(mass, 0, colShape, inertia);
		body->getWorldTransform().setOrigin(pos);
		body->getWorldTransform().setRotation(orn);

		world->addRigidBody(body);

		lua_pushlightuserdata(L, body);
		return 1;
	}
	else
	{
		std::cerr << "Ошибка: неверное число аргументов для createRigidBody, ожидалось 5 (world,shape,mass,pos,orn), но получено " << argc;
	}
	return 0;
}

static i32 gSetBodyPosition(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 3)
	{
		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument needs to be a world";
			return 0;
		}
		if (!lua_isuserdata(L, 2))
		{
			std::cerr << "ошибка: second argument needs to be a body";
			return 0;
		}
		RigidBody* body = (RigidBody*)lua_touserdata(L, 2);
		Vec3 pos = getLuaVectorArg(L, 3);

		Transform2& tr = body->getWorldTransform();
		tr.setOrigin(pos);
		body->setWorldTransform(tr);
	}
	else
	{
		std::cerr << "ошибка: setBodyPosition expects 6 arguments like setBodyPosition(world,body,0,1,0)";
	}
	return 0;
}

static i32 gSetBodyOrientation(lua_State* L)
{
	i32 argc = lua_gettop(L);
	if (argc == 3)
	{
		if (!lua_isuserdata(L, 1))
		{
			std::cerr << "ошибка: first argument needs to be a world";
			return 0;
		}
		if (!lua_isuserdata(L, 2))
		{
			std::cerr << "ошибка: second argument needs to be a body";
			return 0;
		}
		RigidBody* body = (RigidBody*)lua_touserdata(L, 2);
		Quat orn = getLuaQuatArg(L, 3);
		Transform2& tr = body->getWorldTransform();
		tr.setRotation(orn);
		body->setWorldTransform(tr);
	}
	else
	{
		std::cerr << "ошибка: setBodyOrientation expects 3 arguments like setBodyOrientation(world,body,orn)";
	}
	return 0;
}

//b3CreateConvexShape(world, points)

//b3CreateHingeConstraint(world,bodyA,bodyB,...)

static void report_errors(lua_State* L, i32 status)
{
	if (status != 0)
	{
		std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);  // remove error message
	}
}

void LuaPhysicsSetup::initPhysics()
{
	m_guiHelper->setUpAxis(upaxis);
	tukk prefix[] = {"./", "./data/", "../data/", "../../data/", "../../../data/", "../../../../data/"};
	i32 numPrefixes = sizeof(prefix) / sizeof(tukk);
	char relativeFileName[1024];
	FILE* f = 0;
	i32 result = 0;

	for (i32 i = 0; !f && i < numPrefixes; i++)
	{
		sprintf(relativeFileName, "%s%s", prefix[i], sLuaFileName);
		f = fopen(relativeFileName, "rb");
	}
	if (f)
	{
		fclose(f);

		lua_State* L = luaL_newstate();

		luaopen_io(L);  // provides io.*
		luaopen_base(L);
		luaopen_table(L);
		luaopen_string(L);
		luaopen_math(L);
		//luaopen_package(L);
		luaL_openlibs(L);

		// make my_function() available to Lua programs
		lua_register(L, "createDefaultDynamicsWorld", gCreateDefaultDynamicsWorld);
		lua_register(L, "deleteDynamicsWorld", gDeleteDynamicsWorld);
		lua_register(L, "createCubeShape", gCreateCubeShape);
		lua_register(L, "createSphereShape", gCreateSphereShape);
		lua_register(L, "loadMultiBodyFromUrdf", gLoadMultiBodyFromUrdf);

		lua_register(L, "createRigidBody", gCreateRigidBody);
		lua_register(L, "setBodyPosition", gSetBodyPosition);
		lua_register(L, "setBodyOrientation", gSetBodyOrientation);

		i32 s = luaL_loadfile(L, relativeFileName);

		if (s == 0)
		{
			// execute Lua program
			s = lua_pcall(L, 0, LUA_MULTRET, 0);
		}

		report_errors(L, s);
		lua_close(L);
	}
	else
	{
		drx3DError("Cannot find Lua file%s\n", sLuaFileName);
	}
}

void LuaPhysicsSetup::exitPhysics()
{
	CommonMultiBodyBase::exitPhysics();
}

class CommonExampleInterface* LuaDemoCreateFunc(struct CommonExampleOptions& options)
{
	return new LuaPhysicsSetup(options.m_guiHelper);
}
