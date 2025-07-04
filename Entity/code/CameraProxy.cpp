// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/CameraProxy.h>
#include <drx3D/Network/ISerialize.h>

DRXREGISTER_CLASS(CEntityComponentCamera);

//////////////////////////////////////////////////////////////////////////
CEntityComponentCamera::CEntityComponentCamera()
{
	m_componentFlags.Add(EEntityComponentFlags::Legacy);
	m_componentFlags.Add(EEntityComponentFlags::NoSave);
}

//////////////////////////////////////////////////////////////////////////
void CEntityComponentCamera::Initialize()
{
	UpdateMaterialCamera();
}

//////////////////////////////////////////////////////////////////////////
void CEntityComponentCamera::ProcessEvent(const SEntityEvent& event)
{
	switch (event.event)
	{
	case ENTITY_EVENT_INIT:
	case ENTITY_EVENT_XFORM:
		{
			UpdateMaterialCamera();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
uint64 CEntityComponentCamera::GetEventMask() const
{
	return ENTITY_EVENT_BIT(ENTITY_EVENT_XFORM) | ENTITY_EVENT_BIT(ENTITY_EVENT_INIT);
}

//////////////////////////////////////////////////////////////////////////
void CEntityComponentCamera::UpdateMaterialCamera()
{
	float fov = m_camera.GetFov();
	m_camera = GetISystem()->GetViewCamera();
	Matrix34 wtm = m_pEntity->GetWorldTM();
	wtm.OrthonormalizeFast();
	m_camera.SetMatrix(wtm);
	m_camera.SetFrustum(m_camera.GetViewSurfaceX(), m_camera.GetViewSurfaceZ(), fov, m_camera.GetNearPlane(), m_camera.GetFarPlane(), m_camera.GetPixelAspectRatio());

	IMaterial* pMaterial = m_pEntity->GetMaterial();
	if (pMaterial)
		pMaterial->SetCamera(m_camera);
}

//////////////////////////////////////////////////////////////////////////
void CEntityComponentCamera::SetCamera(CCamera& cam)
{
	m_camera = cam;
	UpdateMaterialCamera();
}

//////////////////////////////////////////////////////////////////////////
void CEntityComponentCamera::GameSerialize(TSerialize ser)
{

}
