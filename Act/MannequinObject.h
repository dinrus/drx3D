// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Entity/IEntityComponent.h>

struct IAnimatedCharacter;

class CMannequinObject final : public IEntityComponent
{
private:

	class CProperties final : public IEntityPropertyGroup
	{
	public:

		CProperties(CMannequinObject& owner);

		string            modelFilePath;
		string            actionControllerFilePath;
		string            animationDatabaseFilePath;

		CMannequinObject& ownerBackReference;

	private:

		// IEntityPropertyGroup
		virtual tukk GetLabel() const override;
		virtual void        SerializeProperties(Serialization::IArchive& archive) override;
		// ~IEntityPropertyGroup
	};

public:

	DRX_ENTITY_COMPONENT_INTERFACE_AND_CLASS_GUID(CMannequinObject, "MannequinObjectComponent", "b8be725c-6065-4ec1-afdb-0be2819fbebb"_drx_guid);

	CMannequinObject();

private:

	// IEntityComponent
	virtual void                  Initialize() override;
	virtual void                  ProcessEvent(const SEntityEvent& event) override;
	virtual uint64                GetEventMask() const override;
	virtual IEntityPropertyGroup* GetPropertyGroup() override;
	virtual void                  GetMemoryUsage(IDrxSizer* s) const override;
	// ~IEntityComponent

	void Reset();

private:

	CProperties         m_properties;
	IAnimatedCharacter* m_pAnimatedCharacter;
};
