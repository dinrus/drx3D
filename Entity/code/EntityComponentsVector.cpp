// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/EntityComponentsVector.h>
#include <drx3D/Entity/Entity.h>

SEntityComponentRecord::~SEntityComponentRecord()
{
	Shutdown();
}

void SEntityComponentRecord::Shutdown()
{
	if (pComponent != nullptr)
	{
		static_cast<CEntity*>(pComponent->GetEntity())->ShutDownComponent(*this);
	}
}