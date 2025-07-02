// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//  предварительные объявления.
class CEntityComponentLuaScript;
class CEntity;
struct IScriptTable;
struct SScriptState;

#include <drx3D/Entity/EntityScript.h>

//////////////////////////////////////////////////////////////////////////
// Описание:
//    CScriptProxy object handles all the interaction of the entity with
//    the entity script.
//////////////////////////////////////////////////////////////////////////
class CEntityComponentLuaScript final : public IEntityScriptComponent
{
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentLuaScript, IEntityScriptComponent, "CEntityComponentLuaScript", "38cf87cc-d44b-4a1d-a16d-7ea3c5bde757"_drx_guid);

	CEntityComponentLuaScript();
	virtual ~CEntityComponentLuaScript() override = default;

public:

	virtual void Initialize() final {}

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void   ProcessEvent(const SEntityEvent& event) final;
	virtual uint64 GetEventMask() const final; // Need all events except pre-physics update
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityComponent implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual EEntityProxy GetProxyType() const final { return ENTITY_PROXY_SCRIPT; }
	virtual void         Release()  final           { delete this; };
	virtual void         LegacySerializeXML(XmlNodeRef& entityNode, XmlNodeRef& componentNode, bool bLoading) override final;
	virtual void         GameSerialize(TSerialize ser) final;
	virtual bool         NeedGameSerialize() final;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// IEntityScriptComponent implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void          SetScriptUpdateRate(float fUpdateEveryNSeconds) final { m_fScriptUpdateRate = fUpdateEveryNSeconds; };
	virtual IScriptTable* GetScriptTable() final                                { return m_pThis; };

	virtual void          CallEvent(tukk sEvent) final;
	virtual void          CallEvent(tukk sEvent, float fValue) final;
	virtual void          CallEvent(tukk sEvent, bool bValue) final;
	virtual void          CallEvent(tukk sEvent, tukk sValue) final;
	virtual void          CallEvent(tukk sEvent, EntityId nEntityId) final;
	virtual void          CallEvent(tukk sEvent, const Vec3& vValue) final;

	virtual void          CallInitEvent(bool bFromReload) final;

	virtual void          SendScriptEvent(i32 Event, IScriptTable* pParamters, bool* pRet = nullptr) final;
	virtual void          SendScriptEvent(i32 Event, tukk str, bool* pRet = nullptr) final;
	virtual void          SendScriptEvent(i32 Event, i32 nParam, bool* pRet = nullptr) final;

	virtual void          ChangeScript(IEntityScript* pScript, SEntitySpawnParams* params) final;

	virtual void          EnableScriptUpdate(bool bEnable) final;
	//////////////////////////////////////////////////////////////////////////

	virtual void OnCollision(CEntity* pTarget, i32 matId, const Vec3& pt, const Vec3& n, const Vec3& vel, const Vec3& targetVel, i32 partId, float mass) final;

	//////////////////////////////////////////////////////////////////////////
	// State Management public interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool        GotoState(tukk sStateName) final;
	virtual bool        GotoStateId(i32 nState) final { return GotoState(nState); };
	bool                GotoState(i32 nState);
	virtual bool        IsInState(tukk sStateName) final;
	bool                IsInState(i32 nState);
	virtual tukk GetState() final;
	virtual i32         GetStateId() final;

	virtual void        SetPhysParams(i32 type, IScriptTable* params) final;

	void                SerializeProperties(TSerialize ser);

	virtual void        GetMemoryUsage(IDrxSizer* pSizer) const final;

	bool                IsUpdateEnabled() const { return m_isUpdateEnabled; }

	void                RegisterForAreaEvents(bool enable) { m_enableSoundAreaEvents = enable; }

private:
	SScriptState*  CurrentState() { return m_pScript->GetState(m_currentStateId); }
	void           CreateScriptTable(SEntitySpawnParams* pSpawnParams);
	void           SetEventTargets(XmlNodeRef& eventTargets);
	IScriptSystem* GetIScriptSystem() const { return gEnv->pScriptSystem; }

	void           SerializeTable(TSerialize ser, tukk name);

	void           Update(SEntityUpdateContext& ctx);

private:
	CEntityScript* m_pScript;
	_smart_ptr<IScriptTable> m_pThis;

	float          m_fScriptUpdateTimer;
	float          m_fScriptUpdateRate;

	// Cache Tables.
	SmartScriptTable m_hitTable;

	u8 m_currentStateId;
	// Whether or not the script implemented an OnUpdate function
	u8 m_implementedUpdateFunction : 1;
	// Whether or not the user requested that update be disabled for this script component
	u8 m_isUpdateEnabled : 1;
	u8 m_enableSoundAreaEvents : 1;
};
