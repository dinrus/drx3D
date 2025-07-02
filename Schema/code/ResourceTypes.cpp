// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/Schema/ResourceTypes.h>

#include <drx3D/Schema/CoreAPI.h>
#include <drx3D/Schema/CoreEnv.h>

namespace sxema
{
void RegisterResourceTypes(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(g_resourceModuleGUID);
	{
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(MaterialFileName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(GeomFileName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(SkinName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(CharacterFileName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ParticleEffectName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioEnvironmentName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioPreloadRequestName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioRtpcName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioSwitchName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioSwitchStateName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(AudioTriggerName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(DialogName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(EntityClassName));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ForceFeedbackId));
	}
}
} //endns sxema

DRX_STATIC_AUTO_REGISTER_FUNCTION(&sxema::RegisterResourceTypes)
