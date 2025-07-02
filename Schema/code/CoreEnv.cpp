// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/CoreEnv.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/EnvModule.h>
#include <drx3D/Schema/CoreEnvSignals.h>

namespace sxema
{
void RegisterCoreEnvPackage(IEnvRegistrar& registrar)
{
	RegisterCoreEnvSignals(registrar);

	registrar.RootScope().Register(SXEMA_MAKE_ENV_MODULE(g_stdModuleGUID, "Standard"));
	{
		CEnvRegistrationScope scope = registrar.Scope(g_stdModuleGUID);
		scope.Register(SXEMA_MAKE_ENV_MODULE(g_logModuleGUID, "Log"));
		scope.Register(SXEMA_MAKE_ENV_MODULE(g_mathModuleGUID, "Math"));
		scope.Register(SXEMA_MAKE_ENV_MODULE(g_resourceModuleGUID, "Resource"));

		DrxInvokeStaticCallbacks<sxema::IEnvRegistrar&>(registrar);
		//DrxInvokeStaticCallbacks<i32>(0);
	}
}
} // sxema
