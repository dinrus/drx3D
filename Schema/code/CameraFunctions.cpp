// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Random.h>
#include <drx3D/CoreX/Math/Angle.h>

#include <drx3D/Schema/CoreAPI.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/CoreEnv.h>

static void ReflectType(sxema::CTypeDesc<CCamera>& desc)
{
	desc.SetGUID("{E6C031CA-C81F-4EF6-8853-61B989ACBF76}"_drx_guid);
	desc.SetLabel("Camera");
}

void ReflectType(sxema::CTypeDesc<ray_hit>& desc)
{
	desc.SetGUID("{6691FDBE-13A2-46D9-8601-103C85CC1DEE}"_drx_guid);
	desc.SetLabel("Raycast");
}

namespace sxema
{
namespace Camera
{

CCamera GetCurrentCamera()
{
	return gEnv->pSystem->GetViewCamera();
}

DrxTransform::CRotation GetRotation(const CCamera& camera)
{
	return DrxTransform::CRotation(Matrix33(camera.GetMatrix()));
}

DrxTransform::CTransform GetTransform(const CCamera& camera)
{
	return DrxTransform::CTransform(camera.GetMatrix());
}

Vec3 ScreenToWorld(const Vec2& screenSpaceCoordinates)
{
	Vec3 worldSpaceCoordinates;
	auto& camera = GetISystem()->GetViewCamera();
	gEnv->pRenderer->UnProjectFromScreen(screenSpaceCoordinates.x, camera.GetViewSurfaceZ() - screenSpaceCoordinates.y, 1, &worldSpaceCoordinates.x, &worldSpaceCoordinates.y, &worldSpaceCoordinates.z);
	return worldSpaceCoordinates;
}

static void RegisterFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<CCamera>().GetGUID());
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&GetCurrentCamera, "{0723F8E5-E515-4718-BD7E-8070AD33A5B4}"_drx_guid, "GetCurrentCamera");
		pFunction->BindOutput(0, 'cam', "Camera");
		pFunction->SetDescription("Gets the currently active view camera");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&GetRotation, "{9FDA79CF-2095-4DDB-8F42-7D7ACFBDED42}"_drx_guid, "GetRotation");
		pFunction->BindInput(1, 'cam', "Camera");
		pFunction->BindOutput(0, 'rota', "Rotation");
		pFunction->SetDescription("Gets the specified camera's view rotation");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&GetTransform, "{CB91E621-B12A-44A6-901B-DE6FFB7528BC}"_drx_guid, "GetTransform");
		pFunction->BindInput(1, 'cam', "Camera");
		pFunction->BindOutput(0, 'tran', "Transform");
		pFunction->SetDescription("Gets the specified camera's transform");
		scope.Register(pFunction);
	}
	{
		auto pFunction = SXEMA_MAKE_ENV_FUNCTION(ScreenToWorld, "{4BE156A0-10C2-422E-94E4-7530813FC2A7}"_drx_guid, "ScreenToWorld");
		pFunction->BindInput(1, 'coor', "Screen", "Screen-space position");
		pFunction->BindOutput(0, 'worl', "World", "World-space position");
		pFunction->SetDescription("Unprojects screen-space coordinates to world-space");
		scope.Register(pFunction);
	}
}

} // Camera

namespace Raycast
{
	bool RayCast(const Vec3& origin, const Vec3& direction, Vec3& hitPt, Vec3& hitNormal, ExplicitEntityId& hitEntityId)
	{
		ray_hit hit;
		if (gEnv->pPhysicalWorld->RayWorldIntersection(origin, direction, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &hit, 1, nullptr, 0))
		{
			hitPt = hit.pt;
			hitNormal = hit.n;

			if (IEntity* pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider))
			{
				hitEntityId = ExplicitEntityId(pEntity->GetId());
			}
			else
			{
				hitEntityId = ExplicitEntityId(INVALID_ENTITYID);
			}

			return true;
		}

		return false;
	}

	bool RayCastFromScreen(const Vec2& screenSpaceOrigin, const float maxDistance, Vec3& hitPt, Vec3& hitNormal, ExplicitEntityId& hitEntityId)
	{
		auto& camera = GetISystem()->GetViewCamera();
		float invMouseY = camera.GetViewSurfaceZ() - screenSpaceOrigin.y;

		Vec3 vPos0(0, 0, 0);
		gEnv->pRenderer->UnProjectFromScreen(screenSpaceOrigin.x, (float)invMouseY, 0, &vPos0.x, &vPos0.y, &vPos0.z);

		Vec3 vPos1(0, 0, 0);
		gEnv->pRenderer->UnProjectFromScreen(screenSpaceOrigin.y, (float)invMouseY, 1, &vPos1.x, &vPos1.y, &vPos1.z);

		Vec3 vDir = vPos1 - vPos0;
		vDir.Normalize();

		ray_hit hit;
		if (gEnv->pPhysicalWorld->RayWorldIntersection(vPos0, vDir * maxDistance, ent_all, rwi_stop_at_pierceable | rwi_colltype_any, &hit, 1, nullptr, 0))
		{
			hitPt = hit.pt;
			hitNormal = hit.n;

			if (IEntity* pEntity = gEnv->pEntitySystem->GetEntityFromPhysics(hit.pCollider))
			{
				hitEntityId = ExplicitEntityId(pEntity->GetId());
			}
			else
			{
				hitEntityId = ExplicitEntityId(INVALID_ENTITYID);
			}

			return true;
		}

		return false;
	}

	static void RegisterFunctions(IEnvRegistrar& registrar)
	{
		CEnvRegistrationScope scope = registrar.Scope(GetTypeDesc<ray_hit>().GetGUID());
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&RayCast, "{9875B6AE-7547-483E-A368-91BE0634FA62}"_drx_guid, "RayCast");
			pFunction->BindInput(1, 'orig', "Origin", "The source position to cast from");
			pFunction->BindInput(2, 'dir', "Direction", "The direction and magnitude to cast to");
			pFunction->BindOutput(0, 'hit', "Hit", "Whether or not the ray hit");
			pFunction->BindOutput(3, 'pt', "Position", "The position that we hit");
			pFunction->BindOutput(4, 'norm', "Normal", "The normal of the object that we hit");
			pFunction->BindOutput(5, 'eid', "Entity Id", "The id of the entity that we hit");
			pFunction->SetDescription("Casts a ray through the world's physical grid");
			scope.Register(pFunction);
		}
		{
			auto pFunction = SXEMA_MAKE_ENV_FUNCTION(&RayCastFromScreen, "{ABAAC2E4-895F-44E0-A4FD-4674D9637041}"_drx_guid, "RayCastFromScreen");
			pFunction->BindInput(1, 'orig', "Screen", "The screen-space position to cast from");
			pFunction->BindInput(2, 'maxd', "Max Distance", "Maximum distance from camera");
			pFunction->BindOutput(0, 'hit', "Hit", "Whether or not the ray hit");
			pFunction->BindOutput(3, 'pt', "Position", "The position that we hit");
			pFunction->BindOutput(4, 'norm', "Normal", "The normal of the object that we hit");
			pFunction->BindOutput(5, 'eid', "Entity Id", "The id of the entity that we hit");
			pFunction->SetDescription("Casts a ray through the world's physical grid");
			scope.Register(pFunction);
		}
	}

} // Camera

void RegisterCameraFunctions(IEnvRegistrar& registrar)
{
	CEnvRegistrationScope scope = registrar.Scope(g_mathModuleGUID);
	{
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(CCamera));
		scope.Register(SXEMA_MAKE_ENV_DATA_TYPE(ray_hit));
	}

	Camera::RegisterFunctions(registrar);
	Raycast::RegisterFunctions(registrar);
} // sxema
}

DRX_STATIC_AUTO_REGISTER_FUNCTION(&sxema::RegisterCameraFunctions)
