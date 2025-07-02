// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/Entity.h>
#include <drx3D/Entity/EntitySystem.h>

struct SProximityElement;

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Handles sounds in the entity.
//////////////////////////////////////////////////////////////////////////
class CEntityComponentCamera : public IEntityCameraComponent
{
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentCamera, IEntityCameraComponent, "CEntityComponentCamera", "0f8eee88-f3aa-49b2-a20d-2747b5e33df9"_drx_guid);

	CEntityComponentCamera();
	virtual ~CEntityComponentCamera() {}

public:
	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   Initialize() final;
	virtual void   ProcessEvent(const SEntityEvent& event) final;
	virtual uint64 GetEventMask() const final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetProxyType() const final { return ENTITY_PROXY_CAMERA; }
	virtual void         Release() final            { delete this; };
	virtual void         GameSerialize(TSerialize ser) final;
	virtual bool         NeedGameSerialize() final  { return false; };
	//////////////////////////////////////////////////////////////////////////

	virtual void     SetCamera(CCamera& cam) final;
	virtual CCamera& GetCamera() final { return m_camera; };
	//////////////////////////////////////////////////////////////////////////

	void         UpdateMaterialCamera();

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const final
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	CCamera m_camera;
};
