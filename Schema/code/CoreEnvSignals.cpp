// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/CoreEnvSignals.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/EnvSignal.h>

namespace sxema
{

void SStartSignal::ReflectType(CTypeDesc<SStartSignal>& desc)
{
	desc.SetGUID("a9279137-7c66-491d-b9a0-8752c24c8979"_drx_guid);
	desc.SetLabel("Start");
	desc.SetDescription("Sent when object starts running.");
}

void SStopSignal::ReflectType(CTypeDesc<SStopSignal>& desc)
{
	desc.SetGUID("52ad5add-7781-429b-b4a9-0cb6c905e353"_drx_guid);
	desc.SetLabel("Stop");
	desc.SetDescription("Sent when object stops running.");
}

SUpdateSignal::SUpdateSignal(float _time)
	: time(_time)
{}

void SUpdateSignal::ReflectType(CTypeDesc<SUpdateSignal>& desc)
{
	desc.SetGUID("b2561caa-0753-458b-a91f-e8e38b0f0cdf"_drx_guid);
	desc.SetLabel("Update");
	desc.SetDescription("Sent when object updates.");
	desc.AddMember(&SUpdateSignal::time, 'time', "time", "Time", "Time(s) since last update", 0.0f);
}

void RegisterCoreEnvSignals(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.RootScope();
	{
		scope.Register(SXEMA_MAKE_ENV_SIGNAL(SStartSignal));
		scope.Register(SXEMA_MAKE_ENV_SIGNAL(SStopSignal));
		scope.Register(SXEMA_MAKE_ENV_SIGNAL(SUpdateSignal));
	}
}

} // sxema
