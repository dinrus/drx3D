// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Act/IGameObject.h>

namespace SchematycBaseEnv
{
class CEntityGameObjectExtension : public sxema2::INetworkObject, public CGameObjectExtensionHelper<CEntityGameObjectExtension, IGameObjectExtension>
{
public:

	CEntityGameObjectExtension();

	virtual ~CEntityGameObjectExtension();

	// sxema2::INetworkObject
	virtual bool   RegisterNetworkSerializer(const sxema2::NetworkSerializeCallbackConnection& callbackConnection, i32 aspects) override;
	virtual void   MarkAspectsDirty(i32 aspects) override;
	virtual void   SetAspectDelegation(i32 aspects, const EAspectDelegation delegation) override;
	virtual u16 GetChannelId() const override;
	virtual bool   ServerAuthority() const override;
	virtual bool   ClientAuthority() const override;
	virtual bool   LocalAuthority() const override;
	// ~sxema2::INetworkObject

	// IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject) override;
	virtual void                 InitClient(i32 channelId) override;
	virtual void                 PostInit(IGameObject* pGameObject) override;
	virtual void                 PostInitClient(i32 channelId) override;
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& spawnParams) override;
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& spawnParams) override;

	virtual void                 FullSerialize(TSerialize serialize) override;
	virtual bool                 NetSerialize(TSerialize serialize, EEntityAspects aspects, u8 profile, i32 flags) override;
	virtual void                 PostSerialize() override;
	virtual void                 SerializeSpawnInfo(TSerialize serialize) override;
	virtual ISerializableInfoPtr GetSpawnInfo() override;
	virtual void                 Update(SEntityUpdateContext& context, i32 updateSlot) override;
	virtual void                 PostUpdate(float frameTime) override;
	virtual void                 PostRemoteSpawn() override;
	virtual void                 ProcessEvent(const SEntityEvent& event) override;
	virtual uint64               GetEventMask() const override;
	virtual void                 HandleEvent(const SGameObjectEvent& event) override;
	virtual void                 SetChannelId(u16 channelId) override;
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const override;
	// ~IGameObjectExtension

	static tukk s_szExtensionName;

private:

	enum class EObjectState
	{
		Uninitialized,
		Initialized,
		Running
	};

	void CreateObject(bool bEnteringGame);
	void DestroyObject();
	void RunObject();

	sxema2::SGUID           m_classGUID;
	sxema2::ESimulationMode m_simulationMode;
	EObjectState                m_objectState;
	sxema2::IObject*        m_pObject;
};
}
