// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CAIEntityComponent final : public IEntityComponent
{
public:
	CAIEntityComponent(const CWeakRef<CAIObject>& reference)
		: m_objectReference(reference) {}
	virtual ~CAIEntityComponent();

	// IEntityComponent
	virtual void Initialize() override;

	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const override;
	// ~IEntityComponent

	static void ReflectType(sxema::CTypeDesc<CAIEntityComponent>& desc)
	{
		desc.SetGUID("{435E4CAE-2A4D-453C-BAAB-F3006E329DA7}"_drx_guid);
		desc.SetComponentFlags({ IEntityComponent::EFlags::NoSave });
	}

	tAIObjectID GetAIObjectID() const { return m_objectReference.GetObjectID(); }

protected:
	CWeakRef<CAIObject> m_objectReference;
};