// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CRuntimeAreaObject : public CGameObjectExtensionHelper<CRuntimeAreaObject, IGameObjectExtension>
{
public:

	typedef u32 TSurfaceCRC;

	struct SAudioControls
	{
		DrxAudio::ControlId audioTriggerId;
		DrxAudio::ControlId audioRtpcId;

		explicit SAudioControls(
		  DrxAudio::ControlId _audioTriggerId = DrxAudio::InvalidControlId,
		  DrxAudio::ControlId _audioRtpcId = DrxAudio::InvalidControlId)
			: audioTriggerId(_audioTriggerId)
			, audioRtpcId(_audioRtpcId)
		{}
	};

	typedef std::map<TSurfaceCRC, SAudioControls> TAudioControlMap;

	static TAudioControlMap m_audioControls;

	CRuntimeAreaObject();
	virtual ~CRuntimeAreaObject() override;

	// IGameObjectExtension
	virtual bool                 Init(IGameObject* pGameObject) override;
	virtual void                 InitClient(i32 channelId) override                                                       {}
	virtual void                 PostInit(IGameObject* pGameObject) override                                              {}
	virtual void                 PostInitClient(i32 channelId) override                                                   {}
	virtual bool                 ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override;
	virtual void                 PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params) override {}
	virtual void                 FullSerialize(TSerialize ser) override                                                   {}
	virtual bool                 NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags) override;
	virtual void                 PostSerialize() override                                                                 {}
	virtual void                 SerializeSpawnInfo(TSerialize ser) override                                              {}
	virtual ISerializableInfoPtr GetSpawnInfo() override                                                                  { return NULL; }
	virtual void                 Update(SEntityUpdateContext& ctx, i32 slot) override                                     {}
	virtual void                 HandleEvent(const SGameObjectEvent& gameObjectEvent) override                            {}
	virtual void                 ProcessEvent(const SEntityEvent& entityEvent) override;
	virtual uint64               GetEventMask() const override;
	virtual void                 SetChannelId(u16 id) override                                                         {}
	virtual void                 PostUpdate(float frameTime) override                                                     { DRX_ASSERT(false); }
	virtual void                 PostRemoteSpawn() override                                                               {}
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const override;
	// ~IGameObjectExtension

	// IEntityComponent
	virtual void OnShutDown() override;
	// ~IEntityComponent

private:

	struct SAreaSoundInfo
	{
		SAudioControls audioControls;
		float          parameter;

		explicit SAreaSoundInfo(SAudioControls const& _audioControls, float const _parameter = 0.0f)
			: audioControls(_audioControls)
			, parameter(_parameter)
		{}

		~SAreaSoundInfo()
		{}
	};

	typedef std::map<TSurfaceCRC, SAreaSoundInfo>  TAudioParameterMap;
	typedef std::map<EntityId, TAudioParameterMap> TEntitySoundsMap;

	void UpdateParameterValues(IEntity* const pEntity, TAudioParameterMap& paramMap);
	void StopEntitySounds(EntityId const entityId, TAudioParameterMap& paramMap);

	TEntitySoundsMap m_activeEntitySounds;
};
