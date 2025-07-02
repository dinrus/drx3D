// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/EntitySystem.h>
#include <drx3D/Network/ISerialize.h>

class CEntityComponentTrackViewNode :
	public IEntityComponent
{
public:
	DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS_GUID(CEntityComponentTrackViewNode, "CEntityComponentTrackViewNode", "60f18291-c2a1-46f7-ba7f-02f390c85bb2"_drx_guid);

	CEntityComponentTrackViewNode();
	virtual ~CEntityComponentTrackViewNode() {}

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   Initialize() final;
	virtual void   ProcessEvent(const SEntityEvent& event) final;
	virtual uint64 GetEventMask() const final;
	//////////////////////////////////////////////////////////////////////////

	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const final {};
	virtual EEntityProxy GetProxyType() const final                    { return ENTITY_PROXY_ENTITYNODE; };

	virtual void         Release() final                               { delete this; }
};
