// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Implements base substitution proxy class for entity.
//////////////////////////////////////////////////////////////////////////
struct CEntityComponentSubstitution : IEntitySubstitutionComponent
{
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentSubstitution, IEntitySubstitutionComponent, "CEntityComponentSubstitution", "f60dbb94-8860-494a-9358-6de8c953b324"_drx_guid);

	CEntityComponentSubstitution();
	virtual ~CEntityComponentSubstitution();

public:
	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   Initialize() final {};
	virtual void   ProcessEvent(const SEntityEvent& event) final;
	virtual uint64 GetEventMask() const final;
	;                                           // Need nothing
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetProxyType() const final { return ENTITY_PROXY_SUBSTITUTION; }
	virtual void         Release() final            { delete this; }
	virtual void         GameSerialize(TSerialize ser) final;
	virtual bool         NeedGameSerialize() final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntitySubstitutionComponent interface.
	//////////////////////////////////////////////////////////////////////////
	virtual void         SetSubstitute(IRenderNode* pSubstitute) final;
	virtual IRenderNode* GetSubstitute() final { return m_pSubstitute; }
	//////////////////////////////////////////////////////////////////////////

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const final
	{
		pSizer->AddObject(this, sizeof(*this));
	}

private:
	void Done();
private:
	IRenderNode* m_pSubstitute = nullptr;
};
