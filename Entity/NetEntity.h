// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/Network/INetEntity.h>
#include <array>

class CEntity;

struct SEntitySchedulingProfiles;

class CNetEntity final : public INetEntity
{
public:
	explicit CNetEntity(CEntity* pEntity, const SEntitySpawnParams& params);
	virtual ~CNetEntity();

	virtual bool                  BindToNetwork(EBindToNetworkMode mode) override;
	virtual bool                  BindToNetworkWithParent(EBindToNetworkMode mode, EntityId parentId) override;
	virtual void                  MarkAspectsDirty(NetworkAspectType aspects) override;
	virtual void                  EnableAspect(NetworkAspectType aspects, bool enable) override;
	virtual void                  EnableDelegatableAspect(NetworkAspectType aspects, bool enable) override;

	virtual bool                  CaptureProfileUpr(IGameObjectProfileUpr* pPM) override;
	virtual void                  ReleaseProfileUpr(IGameObjectProfileUpr* pPM) override;
	virtual bool                  HasProfileUpr() override;
	virtual void                  ClearProfileUpr() override;

	virtual void                  SetChannelId(u16 id) override;
	virtual u16                GetChannelId() const override     { return m_channelId; };
	virtual NetworkAspectType     GetEnabledAspects() const override;
	virtual void                  BecomeBound() override            { m_isBoundToNetwork = true; }
	virtual bool                  IsBoundToNetwork() const override { return m_isBoundToNetwork; }
	virtual void                  DontSyncPhysics() override        { m_bNoSyncPhysics = true; }
	virtual bool                  IsAspectDelegatable(NetworkAspectType aspect) const override;

	virtual u8                 GetDefaultProfile(EEntityAspects aspect) override;

	virtual bool                  SetAspectProfile(EEntityAspects aspect, u8 profile, bool fromNetwork) override;
	virtual u8                 GetAspectProfile(EEntityAspects aspect) override;

	virtual void                  SetAuthority(bool auth) override;
	virtual bool                  HasAuthority() const override;
	virtual void                  SetNetworkParent(EntityId id) override;

	virtual bool                  NetSerializeEntity(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) override;

	virtual void                  RmiRegister(const SRmiHandler& handler) override;
	virtual SRmiIndex             RmiByDecoder(SRmiHandler::DecoderF decoder, SRmiHandler** handler) override;
	virtual SRmiHandler::DecoderF RmiByIndex(const SRmiIndex idx) override;

	virtual void                  OnNetworkedEntityTransformChanged(EntityTransformationFlagsMask transformReasons) override;

	virtual void                  OnComponentAddedDuringInitialization(IEntityComponent* pComponent) const override;
	virtual void                  OnEntityInitialized() override;

	static void                   UpdateSchedulingProfiles();

private:
	NetworkAspectType CombineAspects();
	bool              DoSetAspectProfile(EEntityAspects aspect, u8 profile, bool fromNetwork);

private:
	CEntity*                         m_pEntity;
	IGameObjectProfileUpr*       m_pProfileUpr = nullptr;
	const SEntitySchedulingProfiles* m_schedulingProfiles;
	std::vector<SRmiHandler>         m_rmiHandlers;
	TSerialize*                      m_pSpawnSerializer = nullptr;

	NetworkAspectType                m_enabledAspects = NET_ASPECT_ALL;
	NetworkAspectType                m_delegatableAspects = NET_ASPECT_ALL;
	EntityId                         m_cachedParentId = INVALID_ENTITYID;

	u16                           m_channelId = 0;
	u8                            m_isBoundToNetwork    : 1;
	u8                            m_hasAuthority        : 1;
	u8                            m_bNoSyncPhysics      : 1;
	std::array<u8, NUM_ASPECTS>   m_profiles;
};
