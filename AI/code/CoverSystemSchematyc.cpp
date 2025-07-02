// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/CoverSystemSchematyc.h>

#include <drx3D/AI/CoverUserComponent.h>

namespace CoverSystemSchematyc
{
	void Register(sxema::IEnvRegistrar& registrar, sxema::CEnvRegistrationScope& parentScope)
	{
		//Register Components
		CEntityAICoverUserComponent::Register(registrar);
		
		const DrxGUID CoverSystemGUID = "736C4B57-6DEE-405C-9C06-58659FAB999A"_drx_guid;

		parentScope.Register(SXEMA_MAKE_ENV_MODULE(CoverSystemGUID, "Covers"));
		sxema::CEnvRegistrationScope coversScope = registrar.Scope(CoverSystemGUID);

		// Nothing registered now
	}
}
