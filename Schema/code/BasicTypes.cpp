// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/CoreAPI.h>
#include <drx3D/Schema/CoreEnv.h>

namespace sxema
{

void RegisterBasicTypes(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(g_stdModuleGUID);
	{
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(bool));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(i32));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(u32));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(CSharedString));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ObjectId));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ExplicitEntityId));
	}
}

} // sxema

DRX_STATIC_AUTO_REGISTER_FUNCTION(&sxema::RegisterBasicTypes)
