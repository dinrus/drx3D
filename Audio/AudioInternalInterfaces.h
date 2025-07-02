// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/CoreX/String/DrxName.h>

namespace DrxAudio
{
class CATLListener;
class CATLAudioObject;
class CAudioRayInfo;

enum class EAudioRequestType : EnumFlagsType
{
	None,
	AudioUprRequest,
	AudioCallbackUprRequest,
	AudioObjectRequest,
	AudioListenerRequest,
};

enum class EAudioStandaloneFileState : EnumFlagsType
{
	None,
	Playing,
	Stopping,
	Loading,
};

enum class EAudioUprRequestType : EnumFlagsType
{
	None,
	SetAudioImpl,
	ReleaseAudioImpl,
	RefreshAudioSystem,
	StopAllSounds,
	ParseControlsData,
	ParsePreloadsData,
	ClearControlsData,
	ClearPreloadsData,
	PreloadSingleRequest,
	UnloadSingleRequest,
	UnloadAFCMDataByScope,
	DrawDebugInfo,
	AddRequestListener,
	RemoveRequestListener,
	ChangeLanguage,
	RetriggerAudioControls,
	ReleasePendingRays,
	ReloadControlsData,
	GetAudioFileData,
	GetImplInfo,
};

enum class EAudioCallbackUprRequestType : EnumFlagsType
{
	None,
	ReportStartedEvent, //!< Only relevant for delayed playback.
	ReportFinishedEvent,
	ReportFinishedTriggerInstance,
	ReportStartedFile,
	ReportStoppedFile,
	ReportVirtualizedEvent,
	ReportPhysicalizedEvent,
};

enum class EAudioObjectRequestType : EnumFlagsType
{
	None,
	LoadTrigger,
	UnloadTrigger,
	PlayFile,
	StopFile,
	ExecuteTrigger,
	ExecuteTriggerEx,
	StopTrigger,
	StopAllTriggers,
	SetTransformation,
	SetParameter,
	SetSwitchState,
	SetCurrentEnvironments,
	SetEnvironment,
	ResetEnvironments,
	RegisterObject,
	ReleaseObject,
	ProcessPhysicsRay,
	SetName,
};

enum class EAudioListenerRequestType : EnumFlagsType
{
	None,
	SetTransformation,
	RegisterListener,
	ReleaseListener,
	SetName,
};

//////////////////////////////////////////////////////////////////////////
struct SAudioEventListener
{
	SAudioEventListener()
		: pObjectToListenTo(nullptr)
		, OnEvent(nullptr)
		, eventMask(ESystemEvents::None)
	{}

	void const*   pObjectToListenTo;
	void          (* OnEvent)(SRequestInfo const* const);
	ESystemEvents eventMask;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioRequestData : public _i_multithread_reference_target_t
{
	explicit SAudioRequestData(EAudioRequestType const type_)
		: type(type_)
	{}

	virtual ~SAudioRequestData() override = default;

	SAudioRequestData(SAudioRequestData const&) = delete;
	SAudioRequestData(SAudioRequestData&&) = delete;
	SAudioRequestData& operator=(SAudioRequestData const&) = delete;
	SAudioRequestData& operator=(SAudioRequestData&&) = delete;

	EAudioRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioUprRequestDataBase : public SAudioRequestData
{
	explicit SAudioUprRequestDataBase(EAudioUprRequestType const type_)
		: SAudioRequestData(EAudioRequestType::AudioUprRequest)
		, type(type_)
	{}

	virtual ~SAudioUprRequestDataBase() override = default;

	EAudioUprRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioUprRequestType T>
struct SAudioUprRequestData final : public SAudioUprRequestDataBase
{
	SAudioUprRequestData()
		: SAudioUprRequestDataBase(T)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<T> const* const pAMRData)
		: SAudioUprRequestDataBase(T)
	{}

	virtual ~SAudioUprRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::SetAudioImpl> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(Impl::IImpl* const pIImpl_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::SetAudioImpl)
		, pIImpl(pIImpl_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::SetAudioImpl> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::SetAudioImpl)
		, pIImpl(pAMRData->pIImpl)
	{}

	virtual ~SAudioUprRequestData() override = default;

	Impl::IImpl* const pIImpl;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::AddRequestListener> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(
	  void const* const pObjectToListenTo_,
	  void(*func_)(SRequestInfo const* const),
	  ESystemEvents const eventMask_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::AddRequestListener)
		, pObjectToListenTo(pObjectToListenTo_)
		, func(func_)
		, eventMask(eventMask_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::AddRequestListener> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::AddRequestListener)
		, pObjectToListenTo(pAMRData->pObjectToListenTo)
		, func(pAMRData->func)
		, eventMask(pAMRData->eventMask)
	{}

	virtual ~SAudioUprRequestData() override = default;

	void const* const   pObjectToListenTo;
	void                (* func)(SRequestInfo const* const);
	ESystemEvents const eventMask;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::RemoveRequestListener> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(void const* const pObjectToListenTo_, void(*func_)(SRequestInfo const* const))
		: SAudioUprRequestDataBase(EAudioUprRequestType::RemoveRequestListener)
		, pObjectToListenTo(pObjectToListenTo_)
		, func(func_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::RemoveRequestListener> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::RemoveRequestListener)
		, pObjectToListenTo(pAMRData->pObjectToListenTo)
		, func(pAMRData->func)
	{}

	virtual ~SAudioUprRequestData() override = default;

	void const* const pObjectToListenTo;
	void              (* func)(SRequestInfo const* const);
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::ParseControlsData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(char const* const szFolderPath_, EDataScope const dataScope_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ParseControlsData)
		, folderPath(szFolderPath_)
		, dataScope(dataScope_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::ParseControlsData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ParseControlsData)
		, folderPath(pAMRData->folderPath)
		, dataScope(pAMRData->dataScope)
	{}

	virtual ~SAudioUprRequestData() override = default;

	DrxFixedStringT<MaxFilePathLength> const folderPath;
	EDataScope const                         dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::ParsePreloadsData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(char const* const szFolderPath_, EDataScope const dataScope_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ParsePreloadsData)
		, folderPath(szFolderPath_)
		, dataScope(dataScope_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::ParsePreloadsData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ParsePreloadsData)
		, folderPath(pAMRData->folderPath)
		, dataScope(pAMRData->dataScope)
	{}

	virtual ~SAudioUprRequestData() override = default;

	DrxFixedStringT<MaxFilePathLength> const folderPath;
	EDataScope const                         dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::ClearControlsData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(EDataScope const dataScope_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ClearControlsData)
		, dataScope(dataScope_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::ClearControlsData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ClearControlsData)
		, dataScope(pAMRData->dataScope)
	{}

	virtual ~SAudioUprRequestData() override = default;

	EDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::ClearPreloadsData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(EDataScope const dataScope_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ClearPreloadsData)
		, dataScope(dataScope_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::ClearPreloadsData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ClearPreloadsData)
		, dataScope(pAMRData->dataScope)
	{}

	virtual ~SAudioUprRequestData() override = default;

	EDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::PreloadSingleRequest> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(PreloadRequestId const audioPreloadRequestId_, bool const bAutoLoadOnly_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::PreloadSingleRequest)
		, audioPreloadRequestId(audioPreloadRequestId_)
		, bAutoLoadOnly(bAutoLoadOnly_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::PreloadSingleRequest> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::PreloadSingleRequest)
		, audioPreloadRequestId(pAMRData->audioPreloadRequestId)
		, bAutoLoadOnly(pAMRData->bAutoLoadOnly)
	{}

	virtual ~SAudioUprRequestData() override = default;

	PreloadRequestId const audioPreloadRequestId;
	bool const             bAutoLoadOnly;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::UnloadSingleRequest> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(PreloadRequestId const audioPreloadRequestId_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::UnloadSingleRequest)
		, audioPreloadRequestId(audioPreloadRequestId_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::UnloadSingleRequest> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::UnloadSingleRequest)
		, audioPreloadRequestId(pAMRData->audioPreloadRequestId)
	{}

	virtual ~SAudioUprRequestData() override = default;

	PreloadRequestId const audioPreloadRequestId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::UnloadAFCMDataByScope> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(EDataScope const dataScope_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::UnloadAFCMDataByScope)
		, dataScope(dataScope_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::UnloadAFCMDataByScope> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::UnloadAFCMDataByScope)
		, dataScope(pAMRData->dataScope)
	{}

	virtual ~SAudioUprRequestData() override = default;

	EDataScope const dataScope;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::RefreshAudioSystem> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(char const* const szLevelName)
		: SAudioUprRequestDataBase(EAudioUprRequestType::RefreshAudioSystem)
		, levelName(szLevelName)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::RefreshAudioSystem> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::RefreshAudioSystem)
		, levelName(pAMRData->levelName)
	{}

	virtual ~SAudioUprRequestData() override = default;

	DrxFixedStringT<MaxFileNameLength> const levelName;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::ReloadControlsData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(char const* const szFolderPath, char const* const szLevelName)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ReloadControlsData)
		, folderPath(szFolderPath)
		, levelName(szLevelName)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::ReloadControlsData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::ReloadControlsData)
		, folderPath(pAMRData->folderPath)
		, levelName(pAMRData->levelName)
	{}

	virtual ~SAudioUprRequestData() override = default;

	DrxFixedStringT<MaxFilePathLength> const folderPath;
	DrxFixedStringT<MaxFilePathLength> const levelName;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::GetAudioFileData> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(char const* const szName, SFileData& fileData_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::GetAudioFileData)
		, name(szName)
		, fileData(fileData_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::GetAudioFileData> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::GetAudioFileData)
		, name(pAMRData->name)
		, fileData(pAMRData->fileData)
	{}

	virtual ~SAudioUprRequestData() override = default;

	DrxFixedStringT<MaxFileNameLength> const name;
	SFileData&                               fileData;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioUprRequestData<EAudioUprRequestType::GetImplInfo> final : public SAudioUprRequestDataBase
{
	explicit SAudioUprRequestData(SImplInfo& implInfo_)
		: SAudioUprRequestDataBase(EAudioUprRequestType::GetImplInfo)
		, implInfo(implInfo_)
	{}

	explicit SAudioUprRequestData(SAudioUprRequestData<EAudioUprRequestType::GetImplInfo> const* const pAMRData)
		: SAudioUprRequestDataBase(EAudioUprRequestType::GetImplInfo)
		, implInfo(pAMRData->implInfo)
	{}

	virtual ~SAudioUprRequestData() override = default;

	SImplInfo& implInfo;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioCallbackUprRequestDataBase : public SAudioRequestData
{
	explicit SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType const type_)
		: SAudioRequestData(EAudioRequestType::AudioCallbackUprRequest)
		, type(type_)
	{}

	virtual ~SAudioCallbackUprRequestDataBase() override = default;

	EAudioCallbackUprRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioCallbackUprRequestType T>
struct SAudioCallbackUprRequestData final : public SAudioCallbackUprRequestDataBase
{
	SAudioCallbackUprRequestData()
		: SAudioCallbackUprRequestDataBase(T)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<T> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(T)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStartedEvent> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLEvent& audioEvent_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStartedEvent)
		, audioEvent(audioEvent_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStartedEvent> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStartedEvent)
		, audioEvent(pACMRData->audioEvent)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLEvent& audioEvent;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportFinishedEvent> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLEvent& audioEvent_, bool const bSuccess_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportFinishedEvent)
		, audioEvent(audioEvent_)
		, bSuccess(bSuccess_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportFinishedEvent> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportFinishedEvent)
		, audioEvent(pACMRData->audioEvent)
		, bSuccess(pACMRData->bSuccess)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLEvent& audioEvent;
	bool const bSuccess;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportVirtualizedEvent> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLEvent& audioEvent_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportVirtualizedEvent)
		, audioEvent(audioEvent_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportVirtualizedEvent> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportVirtualizedEvent)
		, audioEvent(pACMRData->audioEvent)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLEvent& audioEvent;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportPhysicalizedEvent> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLEvent& audioEvent_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportPhysicalizedEvent)
		, audioEvent(audioEvent_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportPhysicalizedEvent> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportPhysicalizedEvent)
		, audioEvent(pACMRData->audioEvent)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLEvent& audioEvent;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportFinishedTriggerInstance> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(ControlId const audioTriggerId_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportFinishedTriggerInstance)
		, audioTriggerId(audioTriggerId_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportFinishedTriggerInstance> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportFinishedTriggerInstance)
		, audioTriggerId(pACMRData->audioTriggerId)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	ControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStartedFile> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLStandaloneFile& audioStandaloneFile_, bool const bSuccess_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStartedFile)
		, audioStandaloneFile(audioStandaloneFile_)
		, bSuccess(bSuccess_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStartedFile> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStartedFile)
		, audioStandaloneFile(pACMRData->audioStandaloneFile)
		, bSuccess(pACMRData->bSuccess)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLStandaloneFile& audioStandaloneFile;
	bool const          bSuccess;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStoppedFile> final : public SAudioCallbackUprRequestDataBase
{
	explicit SAudioCallbackUprRequestData(CATLStandaloneFile& audioStandaloneFile_)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStoppedFile)
		, audioStandaloneFile(audioStandaloneFile_)
	{}

	explicit SAudioCallbackUprRequestData(SAudioCallbackUprRequestData<EAudioCallbackUprRequestType::ReportStoppedFile> const* const pACMRData)
		: SAudioCallbackUprRequestDataBase(EAudioCallbackUprRequestType::ReportStoppedFile)
		, audioStandaloneFile(pACMRData->audioStandaloneFile)
	{}

	virtual ~SAudioCallbackUprRequestData() override = default;

	CATLStandaloneFile& audioStandaloneFile;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioObjectRequestDataBase : public SAudioRequestData
{
	explicit SAudioObjectRequestDataBase(EAudioObjectRequestType const type_)
		: SAudioRequestData(EAudioRequestType::AudioObjectRequest)
		, type(type_)
	{}

	virtual ~SAudioObjectRequestDataBase() override = default;

	EAudioObjectRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioObjectRequestType T>
struct SAudioObjectRequestData final : public SAudioObjectRequestDataBase
{
	SAudioObjectRequestData()
		: SAudioObjectRequestDataBase(T)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<T> const* const pAORData)
		: SAudioObjectRequestDataBase(T)
	{}

	virtual ~SAudioObjectRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::LoadTrigger> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const audioTriggerId_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::LoadTrigger)
		, audioTriggerId(audioTriggerId_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::LoadTrigger> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::LoadTrigger)
		, audioTriggerId(pAORData->audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::UnloadTrigger> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const audioTriggerId_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::UnloadTrigger)
		, audioTriggerId(audioTriggerId_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::UnloadTrigger> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::UnloadTrigger)
		, audioTriggerId(pAORData->audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::PlayFile> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(
	  DrxFixedStringT<MaxFilePathLength> const& file_,
	  ControlId const usedAudioTriggerId_,
	  bool const bLocalized_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::PlayFile)
		, file(file_)
		, usedAudioTriggerId(usedAudioTriggerId_)
		, bLocalized(bLocalized_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::PlayFile> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::PlayFile)
		, file(pAORData->file)
		, usedAudioTriggerId(pAORData->usedAudioTriggerId)
		, bLocalized(pAORData->bLocalized)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	DrxFixedStringT<MaxFilePathLength> const file;
	ControlId const                          usedAudioTriggerId;
	bool const                               bLocalized;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::StopFile> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(DrxFixedStringT<MaxFilePathLength> const& file_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::StopFile)
		, file(file_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::StopFile> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::StopFile)
		, file(pAORData->file)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	DrxFixedStringT<MaxFilePathLength> const file;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::ExecuteTrigger> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const audioTriggerId_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ExecuteTrigger)
		, audioTriggerId(audioTriggerId_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::ExecuteTrigger> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ExecuteTrigger)
		, audioTriggerId(pAORData->audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::ExecuteTriggerEx> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(SExecuteTriggerData const& data)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ExecuteTriggerEx)
		, name(data.szName)
		, occlusionType(data.occlusionType)
		, transformation(data.transformation)
		, entityId(data.entityId)
		, setCurrentEnvironments(data.setCurrentEnvironments)
		, triggerId(data.triggerId)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::ExecuteTriggerEx> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ExecuteTriggerEx)
		, name(pAORData->name)
		, occlusionType(pAORData->occlusionType)
		, transformation(pAORData->transformation)
		, entityId(pAORData->entityId)
		, setCurrentEnvironments(pAORData->setCurrentEnvironments)
		, triggerId(pAORData->triggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	DrxFixedStringT<MaxObjectNameLength> const name;
	EOcclusionType const                       occlusionType;
	CObjectTransformation const                transformation;
	EntityId const                             entityId;
	bool const setCurrentEnvironments;
	ControlId const                            triggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::StopTrigger> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const audioTriggerId_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::StopTrigger)
		, audioTriggerId(audioTriggerId_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::StopTrigger> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::StopTrigger)
		, audioTriggerId(pAORData->audioTriggerId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const audioTriggerId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetTransformation> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(CObjectTransformation const& transformation_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetTransformation)
		, transformation(transformation_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetTransformation> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetTransformation)
		, transformation(pAORData->transformation)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	CObjectTransformation const transformation;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetParameter> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const parameterId_, float const value_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetParameter)
		, parameterId(parameterId_)
		, value(value_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetParameter> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetParameter)
		, parameterId(pAORData->parameterId)
		, value(pAORData->value)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const parameterId;
	float const     value;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetSwitchState> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(ControlId const audioSwitchId_, SwitchStateId const audioSwitchStateId_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetSwitchState)
		, audioSwitchId(audioSwitchId_)
		, audioSwitchStateId(audioSwitchStateId_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetSwitchState> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetSwitchState)
		, audioSwitchId(pAORData->audioSwitchId)
		, audioSwitchStateId(pAORData->audioSwitchStateId)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	ControlId const     audioSwitchId;
	SwitchStateId const audioSwitchStateId;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetCurrentEnvironments> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(EntityId const entityToIgnore_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetCurrentEnvironments)
		, entityToIgnore(entityToIgnore_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetCurrentEnvironments> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetCurrentEnvironments)
		, entityToIgnore(pAORData->entityToIgnore)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	EntityId const entityToIgnore;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetEnvironment> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(EnvironmentId const audioEnvironmentId_, float const amount_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetEnvironment)
		, audioEnvironmentId(audioEnvironmentId_)
		, amount(amount_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetEnvironment> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetEnvironment)
		, audioEnvironmentId(pAORData->audioEnvironmentId)
		, amount(pAORData->amount)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	EnvironmentId const audioEnvironmentId;
	float const         amount;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::RegisterObject> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(SCreateObjectData const& data)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::RegisterObject)
		, name(data.szName)
		, occlusionType(data.occlusionType)
		, transformation(data.transformation)
		, entityId(data.entityId)
		, setCurrentEnvironments(data.setCurrentEnvironments)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::RegisterObject> const* const pAMRData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::RegisterObject)
		, name(pAMRData->name)
		, occlusionType(pAMRData->occlusionType)
		, transformation(pAMRData->transformation)
		, entityId(pAMRData->entityId)
		, setCurrentEnvironments(pAMRData->setCurrentEnvironments)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	DrxFixedStringT<MaxObjectNameLength> const name;
	EOcclusionType const                       occlusionType;
	CObjectTransformation const                transformation;
	EntityId const                             entityId;
	bool const setCurrentEnvironments;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::ProcessPhysicsRay> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(CAudioRayInfo* const pAudioRayInfo_)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ProcessPhysicsRay)
		, pAudioRayInfo(pAudioRayInfo_)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::ProcessPhysicsRay> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::ProcessPhysicsRay)
		, pAudioRayInfo(pAORData->pAudioRayInfo)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	CAudioRayInfo* const pAudioRayInfo;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioObjectRequestData<EAudioObjectRequestType::SetName> final : public SAudioObjectRequestDataBase
{
	explicit SAudioObjectRequestData(char const* const szName)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetName)
		, name(szName)
	{}

	explicit SAudioObjectRequestData(SAudioObjectRequestData<EAudioObjectRequestType::SetName> const* const pAORData)
		: SAudioObjectRequestDataBase(EAudioObjectRequestType::SetName)
		, name(pAORData->name)
	{}

	virtual ~SAudioObjectRequestData() override = default;

	DrxFixedStringT<MaxObjectNameLength> const name;
};

//////////////////////////////////////////////////////////////////////////
struct SAudioListenerRequestDataBase : public SAudioRequestData
{
	explicit SAudioListenerRequestDataBase(EAudioListenerRequestType const type_)
		: SAudioRequestData(EAudioRequestType::AudioListenerRequest)
		, type(type_)
	{}

	virtual ~SAudioListenerRequestDataBase() override = default;

	EAudioListenerRequestType const type;
};

//////////////////////////////////////////////////////////////////////////
template<EAudioListenerRequestType T>
struct SAudioListenerRequestData final : public SAudioListenerRequestDataBase
{
	SAudioListenerRequestData()
		: SAudioListenerRequestDataBase(T)
	{}

	explicit SAudioListenerRequestData(SAudioListenerRequestData<T> const* const pALRData)
		: SAudioListenerRequestDataBase(T)
	{}

	virtual ~SAudioListenerRequestData() override = default;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestData<EAudioListenerRequestType::SetTransformation> final : public SAudioListenerRequestDataBase
{
	explicit SAudioListenerRequestData(
	  CObjectTransformation const& transformation_,
	  CATLListener* const pListener_)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::SetTransformation)
		, transformation(transformation_)
		, pListener(pListener_)
	{}

	explicit SAudioListenerRequestData(SAudioListenerRequestData<EAudioListenerRequestType::SetTransformation> const* const pALRData)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::SetTransformation)
		, transformation(pALRData->transformation)
		, pListener(pALRData->pListener)
	{}

	virtual ~SAudioListenerRequestData() override = default;

	CObjectTransformation const transformation;
	CATLListener* const         pListener;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestData<EAudioListenerRequestType::RegisterListener> final : public SAudioListenerRequestDataBase
{
	explicit SAudioListenerRequestData(CATLListener** const ppListener_, char const* const szName)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::RegisterListener)
		, ppListener(ppListener_)
		, name(szName)
	{}

	explicit SAudioListenerRequestData(SAudioListenerRequestData<EAudioListenerRequestType::RegisterListener> const* const pALRData)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::RegisterListener)
		, ppListener(pALRData->ppListener)
		, name(pALRData->name)
	{}

	virtual ~SAudioListenerRequestData() override = default;

	CATLListener** const                       ppListener;
	DrxFixedStringT<MaxObjectNameLength> const name;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestData<EAudioListenerRequestType::ReleaseListener> final : public SAudioListenerRequestDataBase
{
	explicit SAudioListenerRequestData(CATLListener* const pListener_)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::ReleaseListener)
		, pListener(pListener_)
	{}

	explicit SAudioListenerRequestData(SAudioListenerRequestData<EAudioListenerRequestType::ReleaseListener> const* const pALRData)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::ReleaseListener)
		, pListener(pALRData->pListener)
	{}

	virtual ~SAudioListenerRequestData() override = default;

	CATLListener* const pListener;
};

//////////////////////////////////////////////////////////////////////////
template<>
struct SAudioListenerRequestData<EAudioListenerRequestType::SetName> final : public SAudioListenerRequestDataBase
{
	explicit SAudioListenerRequestData(char const* const szName, CATLListener* const pListener_)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::SetName)
		, pListener(pListener_)
		, name(szName)
	{}

	explicit SAudioListenerRequestData(SAudioListenerRequestData<EAudioListenerRequestType::SetName> const* const pALRData)
		: SAudioListenerRequestDataBase(EAudioListenerRequestType::SetName)
		, pListener(pALRData->pListener)
		, name(pALRData->name)
	{}

	virtual ~SAudioListenerRequestData() override = default;

	CATLListener* const                        pListener;
	DrxFixedStringT<MaxObjectNameLength> const name;
};

SAudioRequestData* AllocateRequestData(SAudioRequestData const* const pRequestData);

class CAudioRequest
{
public:

	CAudioRequest() = default;

	explicit CAudioRequest(SAudioRequestData const* const pRequestData)
		: flags(ERequestFlags::None)
		, pObject(nullptr)
		, pOwner(nullptr)
		, pUserData(nullptr)
		, pUserDataOwner(nullptr)
		, status(ERequestStatus::None)
		, pData(AllocateRequestData(pRequestData))
	{}

	explicit CAudioRequest(
	  ERequestFlags const flags_,
	  CATLAudioObject* const pObject_,
	  uk const pOwner_,
	  uk const pUserData_,
	  uk const pUserDataOwner_,
	  SAudioRequestData const* const pRequestData_)
		: flags(flags_)
		, pObject(pObject_)
		, pOwner(pOwner_)
		, pUserData(pUserData_)
		, pUserDataOwner(pUserDataOwner_)
		, status(ERequestStatus::None)
		, pData(AllocateRequestData(pRequestData_))
	{}

	SAudioRequestData* GetData() const { return pData.get(); }

	ERequestFlags    flags = ERequestFlags::None;
	CATLAudioObject* pObject = nullptr;
	uk            pOwner = nullptr;
	uk            pUserData = nullptr;
	uk            pUserDataOwner = nullptr;
	ERequestStatus   status = ERequestStatus::None;

private:

	// Must be private as it needs "AllocateRequestData"!
	_smart_ptr<SAudioRequestData> pData = nullptr;
};

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
// Filter for drawing debug info to the screen
enum class EAudioDebugDrawFilter : EnumFlagsType
{
	All                        = 0,
	ShowSpheres                = BIT(6),  // a
	ShowObjectLabel            = BIT(7),  // b
	ShowObjectTriggers         = BIT(8),  // c
	ShowObjectStates           = BIT(9),  // d
	ShowObjectParameters       = BIT(10), // e
	ShowObjectEnvironments     = BIT(11), // f
	ShowObjectDistance         = BIT(12), // g
	ShowOcclusionRayLabels     = BIT(13), // h
	ShowOcclusionRays          = BIT(14), // i
	DrawListenerOcclusionPlane = BIT(15), // j
	ShowObjectStandaloneFiles  = BIT(16), // k

	HideMemoryInfo             = BIT(18), // m
	FilterAllObjectInfo        = BIT(19), // n

	ShowStandaloneFiles        = BIT(26), // u
	ShowActiveEvents           = BIT(27), // v
	ShowActiveObjects          = BIT(28), // w
	ShowFileCacheUprInfo   = BIT(29), // x
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EAudioDebugDrawFilter);

static constexpr EAudioDebugDrawFilter objectDebugMask =
  EAudioDebugDrawFilter::ShowSpheres |
  EAudioDebugDrawFilter::ShowObjectLabel |
  EAudioDebugDrawFilter::ShowObjectTriggers |
  EAudioDebugDrawFilter::ShowObjectStates |
  EAudioDebugDrawFilter::ShowObjectParameters |
  EAudioDebugDrawFilter::ShowObjectEnvironments |
  EAudioDebugDrawFilter::ShowObjectDistance |
  EAudioDebugDrawFilter::ShowOcclusionRayLabels |
  EAudioDebugDrawFilter::ShowOcclusionRays |
  EAudioDebugDrawFilter::DrawListenerOcclusionPlane |
  EAudioDebugDrawFilter::ShowObjectStandaloneFiles;

#endif // INCLUDE_AUDIO_PRODUCTION_CODE

} //endns DrxAudio
