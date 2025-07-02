// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/InternalEntities.h>
#include <drx3D/Audio/AudioListenerUpr.h>
#include <drx3D/Audio/AudioEventListenerUpr.h>
#include <drx3D/Audio/AudioStandaloneFileUpr.h>
#include <drx3D/Audio/AudioEventUpr.h>
#include <drx3D/Audio/AudioObjectUpr.h>
#include <drx3D/Audio/AudioXMLProcessor.h>
#include <drx3D/Input/IInput.h>

namespace DrxAudio
{
enum class EInternalStates : EnumFlagsType
{
	None                        = 0,
	AudioMiddlewareShuttingDown = BIT(0),
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EInternalStates);

class CSystem;

class CAudioTranslationLayer final : public IInputEventListener
{
public:

	CAudioTranslationLayer();
	virtual ~CAudioTranslationLayer() override;

	CAudioTranslationLayer(CAudioTranslationLayer const&) = delete;
	CAudioTranslationLayer(CAudioTranslationLayer&&) = delete;
	CAudioTranslationLayer& operator=(CAudioTranslationLayer const&) = delete;
	CAudioTranslationLayer& operator=(CAudioTranslationLayer&&) = delete;

	// IInputEventListener
	virtual bool OnInputEvent(SInputEvent const& event) override;
	// ~IInputEventListener

	bool           Initialize(CSystem* const pAudioSystem);
	bool           ShutDown();
	void           ProcessRequest(CAudioRequest& request);
	void           Update(float const deltaTime);

	bool           CanProcessRequests() const { return (m_flags& EInternalStates::AudioMiddlewareShuttingDown) == 0; }

	ERequestStatus ParseControlsData(char const* const szFolderPath, EDataScope const dataScope);
	ERequestStatus ClearControlsData(EDataScope const dataScope);
	ERequestStatus ParsePreloadsData(char const* const szFolderPath, EDataScope const dataScope);
	ERequestStatus ClearPreloadsData(EDataScope const dataScope);

	void           NotifyListener(CAudioRequest const& request);

	void           IncrementGlobalObjectSyncCallbackCounter();
	void           DecrementGlobalObjectSyncCallbackCounter();

	char const*    GetConfigPath() const;

private:

	ERequestStatus ProcessAudioUprRequest(CAudioRequest const& request);
	ERequestStatus ProcessAudioCallbackUprRequest(CAudioRequest& request);
	ERequestStatus ProcessAudioObjectRequest(CAudioRequest const& request);
	ERequestStatus ProcessAudioListenerRequest(SAudioRequestData const* const pPassedRequestData);
	ERequestStatus SetImpl(Impl::IImpl* const pIImpl);
	void           ReleaseImpl();

	ERequestStatus RefreshAudioSystem(char const* const szLevelName);
	void           SetImplLanguage();
	void           CreateInternalControls();
	void           SetCurrentEnvironmentsOnObject(CATLAudioObject* const pObject, EntityId const entityToIgnore);

	void           CreateInternalTrigger(char const* const szTriggerName, ControlId const triggerId, CATLTriggerImpl const* const pTriggerImpl);
	void           CreateInternalSwitch(char const* const szSwitchName, ControlId const switchId, std::vector<char const*> const& stateNames);

	// ATLObject containers
	AudioTriggerLookup        m_triggers;
	AudioParameterLookup      m_parameters;
	AudioSwitchLookup         m_switches;
	AudioPreloadRequestLookup m_preloadRequests;
	AudioEnvironmentLookup    m_environments;

	CATLAudioObject*          m_pGlobalAudioObject = nullptr;

	// Components
	CAudioStandaloneFileUpr m_audioStandaloneFileMgr;
	CAudioEventUpr          m_audioEventMgr;
	CAudioObjectUpr         m_audioObjectMgr;
	CAudioListenerUpr       m_audioListenerMgr;
	CFileCacheUpr           m_fileCacheMgr;
	CAudioEventListenerUpr  m_audioEventListenerMgr;
	CAudioXMLProcessor          m_xmlProcessor;

	SInternalControls           m_internalControls;

	// Utility members
	EInternalStates                    m_flags = EInternalStates::None;
	Impl::IImpl*                       m_pIImpl = nullptr;
	SImplInfo                          m_implInfo;
	DrxFixedStringT<MaxFilePathLength> m_configPath;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
public:

	void DrawAudioSystemDebugInfo();
	void GetAudioTriggerData(ControlId const audioTriggerId, STriggerData& audioTriggerData) const;

private:

	void DrawAudioObjectDebugInfo(IRenderAuxGeom& auxGeom);
	void DrawATLComponentDebugInfo(IRenderAuxGeom& auxGeom, float posX, float const posY);
	void RetriggerAudioControls();
#endif // INCLUDE_AUDIO_PRODUCTION_CODE
};
} //endns DrxAudio
