// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Serialization/Math.h>

#include <drx3D/Schema/CoreAPI.h>
#include <drx3D/Schema/CoreEnv.h>

namespace sxema
{

void RegisterMathTypes(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(g_mathModuleGUID);
	{
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(float));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(Vec2));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(Vec3));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ColorF));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ColorB));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(DrxTransform::CRotation));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(DrxTransform::CAngle));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(DrxTransform::CAngles3));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(DrxTransform::CTransform));
	}
}

} // sxema

DRX_STATIC_AUTO_REGISTER_FUNCTION(&sxema::RegisterMathTypes)
