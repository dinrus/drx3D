// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/EntityUtilsComponent.h>

#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

#include <drx3D/Schema/EnvFunction.h>
#include <drx3D/Schema/SharedString.h>
#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/EnvComponent.h>
#include <drx3D/Schema/EnvSignal.h>

#include  <drx3D/Schema/IEnvRegistrar.h>

namespace sxema
{
void CEntityUtilsComponent::ReflectType(CTypeDesc<CEntityUtilsComponent>& desc)
{
	desc.SetGUID("e88093df-904f-4c52-af38-911e26777cdc"_drx_guid);
	desc.SetLabel("Entity");
	desc.SetDescription("Entity utilities component");
	desc.SetIcon("icons:schematyc/entity_utils_component.ico");
	desc.SetComponentFlags({ EFlags::Singleton, EFlags::HideFromInspector, EFlags::HiddenFromUser });
}

void CEntityUtilsComponent::Register(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		CEnvRegistrationScope componentScope = scope.Register(SXEMA_MAKE_ENV_COMPONENT(CEntityUtilsComponent));
	}
}

} // sxema
