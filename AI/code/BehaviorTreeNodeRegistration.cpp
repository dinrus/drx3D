// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/BehaviorTreeNodeRegistration.h>

#include <drx3D/AI/BehaviorTreeNodes_Core.h>
#include <drx3D/AI/BehaviorTreeNodes_AI.h>
#include <drx3D/AI/BehaviorTreeNodes_Helicopter.h>
#include <drx3D/AI/BehaviorTreeNodes_Basic.h>

namespace BehaviorTree
{
void RegisterBehaviorTreeNodes()
{
	RegisterBehaviorTreeNodes_Core();
	RegisterBehaviorTreeNodes_AI();
	RegisterBehaviorTreeNodesHelicopter();
	RegisterBehaviorTreeNodes_Basic();
}
}
