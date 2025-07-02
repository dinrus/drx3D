#include "../BasicExample.h"

#include <drx3D/Common/Interfaces/CommonExampleInterface.h>
#include <drx3D/Common/Interfaces/CommonGUIHelperInterface.h>
#include <drx3D/Physics/Collision/Dispatch/CollisionObject2.h>
#include <drx3D/Physics/Collision/Shapes/CollisionShape.h>
#include <drx3D/Physics/Dynamics/DiscreteDynamicsWorld.h>

#include <drx3D/Maths/Linear/Transform2.h>
#include <drx3D/Maths/Linear/HashMap.h>

i32 main(i32 argc, tuk argv[])
{
	DummyGUIHelper noGfx;

	CommonExampleOptions options(&noGfx);
	CommonExampleInterface* example = BasicExampleCreateFunc(options);

	example->initPhysics();
	example->stepSimulation(1.f / 60.f);
	example->exitPhysics();

	delete example;

	return 0;
}
