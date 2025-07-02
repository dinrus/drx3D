// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IEntityBehaviorTreeComponent.h>

// *INDENT-OFF* - <hard to read code and declarations due to inconsistent indentation>

class CEntityAIBehaviorTreeComponent final : public IEntityBehaviorTreeComponent
{
private:

	// - just a wrapper around a file name without extension
	// - has a custom serializer that offers the user all available behavior trees as a UI list
	class CBehaviorTreeName
	{
	public:

		static void       ReflectType(sxema::CTypeDesc<CBehaviorTreeName>& desc);
		bool              Serialize(Serialization::IArchive& archive);
		tukk       GetFilePathWithoutExtension() const { return m_filePathWithoutExtension.c_str(); }
		bool              operator==(const CBehaviorTreeName& rhs) const { return m_filePathWithoutExtension == rhs.m_filePathWithoutExtension; }

	private:

		string            m_filePathWithoutExtension;
	};

public:

	CEntityAIBehaviorTreeComponent();

	// IEntityComponent
	virtual void          OnShutDown() override;
	virtual uint64        GetEventMask() const override;
	virtual void          ProcessEvent(const SEntityEvent& event) override;
	// ~IEntityComponent

	// IEntityBehaviorTreeComponent
	virtual bool IsRunning() const override { return m_bBehaviorTreeIsRunning; }
	virtual void SendEvent(tukk szEventName) override;
	// ~IEntityBehaviorTreeComponent

	static void           ReflectType(sxema::CTypeDesc<CEntityAIBehaviorTreeComponent>& desc);
	static void           Register(sxema::IEnvRegistrar& registrar);

private:

	void                  EnsureBehaviorTreeIsRunning();
	void                  EnsureBehaviorTreeIsStopped();

	void                  SchematycFunction_SendEvent(const sxema::CSharedString& eventName);
	template <class TValue>
	void                  SchematycFunction_SetBBKeyValue(const sxema::CSharedString& key, const TValue& value);

private:

	CBehaviorTreeName     m_behaviorTreeName;
	bool                  m_bBehaviorTreeShouldBeRunning;
	bool                  m_bBehaviorTreeIsRunning;
};
