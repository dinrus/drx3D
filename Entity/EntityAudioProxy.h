// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Audio/IObject.h>
#include <drx3D/CoreX/Memory/STLPoolAllocator.h>
#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Handles audio on the entity.
//////////////////////////////////////////////////////////////////////////
class CEntityComponentAudio final : public IEntityAudioComponent
{
public:
	DRX_ENTITY_COMPONENT_CLASS_GUID(CEntityComponentAudio, IEntityAudioComponent, "CEntityComponentAudio", "51ae5fc2-1b45-4351-ac88-9caf0c757b5f"_drx_guid);

	CEntityComponentAudio();
	virtual ~CEntityComponentAudio() override;

	// IEntityComponent
	virtual void         ProcessEvent(const SEntityEvent& event) override;
	virtual void         Initialize() override;
	virtual uint64       GetEventMask() const override;
	virtual EEntityProxy GetProxyType() const override                    { return ENTITY_PROXY_AUDIO; }
	virtual void         GameSerialize(TSerialize ser) override;
	virtual bool         NeedGameSerialize() override                     { return false; }
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const override { pSizer->AddObject(this, sizeof(*this)); }
	virtual void		 OnTransformChanged() override;
	// ~IEntityComponent

	// IEntityAudioComponent
	virtual void                    SetFadeDistance(float const fadeDistance) override                       { m_fadeDistance = fadeDistance; }
	virtual float                   GetFadeDistance() const override                                         { return m_fadeDistance; }
	virtual void                    SetEnvironmentFadeDistance(float const environmentFadeDistance) override { m_environmentFadeDistance = environmentFadeDistance; }
	virtual float                   GetEnvironmentFadeDistance() const override                              { return m_environmentFadeDistance; }
	virtual float                   GetGreatestFadeDistance() const override;
	virtual void                    SetEnvironmentId(DrxAudio::EnvironmentId const environmentId) override   { m_environmentId = environmentId; }
	virtual DrxAudio::EnvironmentId GetEnvironmentId() const override                                        { return m_environmentId; }
	virtual DrxAudio::AuxObjectId   CreateAudioAuxObject() override;
	virtual bool                    RemoveAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId) override;
	virtual void                    SetAudioAuxObjectOffset(Matrix34 const& offset, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual Matrix34 const&         GetAudioAuxObjectOffset(DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual bool                    PlayFile(DrxAudio::SPlayFileInfo const& playbackInfo, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) override;
	virtual void                    StopFile(char const* const szFile, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual bool                    ExecuteTrigger(DrxAudio::ControlId const audioTriggerId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) override;
	virtual void                    StopTrigger(DrxAudio::ControlId const audioTriggerId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId, DrxAudio::SRequestUserData const& userData = DrxAudio::SRequestUserData::GetEmptyObject()) override;
	virtual void                    SetSwitchState(DrxAudio::ControlId const audioSwitchId, DrxAudio::SwitchStateId const audioStateId, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual void                    SetParameter(DrxAudio::ControlId const parameterId, float const value, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual void                    SetObstructionCalcType(DrxAudio::EOcclusionType const occlusionType, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual void                    SetEnvironmentAmount(DrxAudio::EnvironmentId const audioEnvironmentId, float const amount, DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual void                    SetCurrentEnvironments(DrxAudio::AuxObjectId const audioAuxObjectId = DrxAudio::DefaultAuxObjectId) override;
	virtual void                    AudioAuxObjectsMoveWithEntity(bool const bCanMoveWithEntity) override;
	virtual void                    AddAsListenerToAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId, void (* func)(DrxAudio::SRequestInfo const* const), DrxAudio::ESystemEvents const eventMask) override;
	virtual void                    RemoveAsListenerFromAudioAuxObject(DrxAudio::AuxObjectId const audioAuxObjectId, void (* func)(DrxAudio::SRequestInfo const* const)) override;
	virtual DrxAudio::AuxObjectId   GetAuxObjectIdFromAudioObject(DrxAudio::IObject* pObject) override;
	// ~IEntityAudioComponent

private:

	enum EEntityAudioProxyFlags : DrxAudio::EnumFlagsType
	{
		eEntityAudioProxyFlags_None = 0,
		eEntityAudioProxyFlags_CanMoveWithEntity = BIT(0),
	};

	struct SAuxObjectWrapper
	{
		explicit SAuxObjectWrapper(DrxAudio::IObject* const _pIObject)
			: pIObject(_pIObject)
			, offset(IDENTITY)
		{}

		~SAuxObjectWrapper() = default;

		DrxAudio::IObject* const pIObject;
		Matrix34                 offset;

	private:

		SAuxObjectWrapper()
			: pIObject(nullptr)
			, offset(IDENTITY)
		{}
	};

	using AuxObjectPair = std::pair<DrxAudio::AuxObjectId const, SAuxObjectWrapper>;

	typedef std::map<DrxAudio::AuxObjectId const, SAuxObjectWrapper, std::less<DrxAudio::AuxObjectId>, stl::STLPoolAllocator<AuxObjectPair>> AuxObjects;

	static AuxObjectPair s_nullAuxObjectPair;

	void           OnListenerEnter(CEntity const* const pEntity);
	void           OnListenerMoveNear(Vec3 const& closestPointToArea);
	void           OnListenerMoveInside(Vec3 const& listenerPos);
	void           OnListenerExclusiveMoveInside(CEntity const* const __restrict pEntity, CEntity const* const __restrict pAreaHigh, CEntity const* const __restrict pAreaLow, float const fade);
	void           OnMove();
	AuxObjectPair& GetAudioAuxObjectPair(DrxAudio::AuxObjectId const audioAuxObjectId);
	void           SetEnvironmentAmountInternal(CEntity const* const pIEntity, float const amount) const;

	// Function objects
	struct SReleaseAudioProxy
	{
		inline void operator()(AuxObjectPair const& pair)
		{
			gEnv->pAudioSystem->ReleaseObject(pair.second.pIObject);
		}
	};

	struct SRepositionAudioProxy
	{
		SRepositionAudioProxy(Matrix34 const& _transformation, DrxAudio::SRequestUserData const& _userData)
			: transformation(_transformation)
			, userData(_userData)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetTransformation(transformation * pair.second.offset, userData);
		}

	private:

		Matrix34 const&                   transformation;
		DrxAudio::SRequestUserData const& userData;
	};

	struct SPlayFile
	{
		SPlayFile(DrxAudio::SPlayFileInfo const& _playbackInfo, DrxAudio::SRequestUserData const& _userData)
			: playbackInfo(_playbackInfo)
			, userData(_userData)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->PlayFile(playbackInfo, userData);
		}

	private:

		DrxAudio::SPlayFileInfo const&    playbackInfo;
		DrxAudio::SRequestUserData const& userData;
	};

	struct SStopFile
	{
		explicit SStopFile(char const* const _szFile)
			: szFile(_szFile)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->StopFile(szFile);
		}

	private:

		char const* const szFile;
	};

	struct SStopTrigger
	{
		SStopTrigger(DrxAudio::ControlId const _audioTriggerId, DrxAudio::SRequestUserData const& _userData)
			: audioTriggerId(_audioTriggerId)
			, userData(_userData)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->StopTrigger(audioTriggerId, userData);
		}

	private:

		DrxAudio::ControlId const         audioTriggerId;
		DrxAudio::SRequestUserData const& userData;
	};

	struct SSetSwitchState
	{
		SSetSwitchState(DrxAudio::ControlId const _audioSwitchId, DrxAudio::SwitchStateId const _audioStateId)
			: audioSwitchId(_audioSwitchId)
			, audioStateId(_audioStateId)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetSwitchState(audioSwitchId, audioStateId);
		}

	private:

		DrxAudio::ControlId const     audioSwitchId;
		DrxAudio::SwitchStateId const audioStateId;
	};

	struct SSetParameter
	{
		SSetParameter(DrxAudio::ControlId const _parameterId, float const _value)
			: parameterId(_parameterId)
			, value(_value)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetParameter(parameterId, value);
		}

	private:

		DrxAudio::ControlId const parameterId;
		float const               value;
	};

	struct SSetOcclusionType
	{
		explicit SSetOcclusionType(DrxAudio::EOcclusionType const _occlusionType)
			: occlusionType(_occlusionType)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetOcclusionType(occlusionType);
		}

	private:

		DrxAudio::EOcclusionType const occlusionType;
	};

	struct SSetEnvironmentAmount
	{
		SSetEnvironmentAmount(DrxAudio::EnvironmentId const _audioEnvironmentId, float const _amount)
			: audioEnvironmentId(_audioEnvironmentId)
			, amount(_amount)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetEnvironment(audioEnvironmentId, amount);
		}

	private:

		DrxAudio::EnvironmentId const audioEnvironmentId;
		float const                   amount;
	};

	struct SSetCurrentEnvironments
	{
		explicit SSetCurrentEnvironments(EntityId const entityId)
			: m_entityId(entityId)
		{}

		inline void operator()(AuxObjectPair const& pair)
		{
			pair.second.pIObject->SetCurrentEnvironments(m_entityId);
		}

	private:

		EntityId const m_entityId;
	};

	struct SSetAuxAudioProxyOffset
	{
		SSetAuxAudioProxyOffset(Matrix34 const& _offset, Matrix34 const& _entityPosition)
			: offset(_offset)
			, entityPosition(_entityPosition)
		{}

		inline void operator()(AuxObjectPair& pair)
		{
			pair.second.offset = offset;
			(SRepositionAudioProxy(entityPosition, DrxAudio::SRequestUserData::GetEmptyObject()))(pair);
		}

	private:

		Matrix34 const& offset;
		Matrix34 const& entityPosition;
	};
	// ~Function objects

	AuxObjects              m_mapAuxObjects;
	DrxAudio::AuxObjectId   m_auxObjectIdCounter;

	DrxAudio::EnvironmentId m_environmentId;
	DrxAudio::EnumFlagsType m_flags;

	float                   m_fadeDistance;
	float                   m_environmentFadeDistance;
};
