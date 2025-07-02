// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/ATLEntities.h>
#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>

namespace DrxAudio
{
class CATLAudioObject;
class CAudioEventUpr;
class CAudioStandaloneFileUpr;
class CAudioListenerUpr;

namespace Impl
{
struct SObject3DAttributes;
}

class CAudioObjectUpr final
{
public:

	using ConstructedAudioObjectsList = std::list<CATLAudioObject*>;

	explicit CAudioObjectUpr(
	  CAudioEventUpr& audioEventMgr,
	  CAudioStandaloneFileUpr& audioStandaloneFileMgr,
	  CAudioListenerUpr const& listenerUpr);
	~CAudioObjectUpr();

	CAudioObjectUpr(CAudioObjectUpr const&) = delete;
	CAudioObjectUpr(CAudioObjectUpr&&) = delete;
	CAudioObjectUpr& operator=(CAudioObjectUpr const&) = delete;
	CAudioObjectUpr& operator=(CAudioObjectUpr&&) = delete;

	void                 Init(Impl::IImpl* const pIImpl);
	void                 Release();
	void                 Update(float const deltaTime, Impl::SObject3DAttributes const& listenerAttributes);
	void                 RegisterObject(CATLAudioObject* const pObject);

	void                 ReportStartedEvent(CATLEvent* const pEvent);
	void                 ReportFinishedEvent(CATLEvent* const pEvent, bool const bSuccess);
	void                 GetStartedStandaloneFileRequestData(CATLStandaloneFile* const pStandaloneFile, CAudioRequest& request);
	void                 ReportFinishedStandaloneFile(CATLStandaloneFile* const pStandaloneFile);
	void                 ReleasePendingRays();
	bool                 IsActive(CATLAudioObject const* const pAudioObject) const;

#if defined(INCLUDE_AUDIO_PRODUCTION_CODE)
	size_t                             GetNumAudioObjects() const;
	size_t                             GetNumActiveAudioObjects() const;
	ConstructedAudioObjectsList const& GetAudioObjects() const { return m_constructedAudioObjects; }
	void                               DrawPerObjectDebugInfo(
	  IRenderAuxGeom& auxGeom,
	  Vec3 const& listenerPos,
	  AudioTriggerLookup const& triggers,
	  AudioParameterLookup const& parameters,
	  AudioSwitchLookup const& switches,
	  AudioPreloadRequestLookup const& preloadRequests,
	  AudioEnvironmentLookup const& environments) const;
	void DrawDebugInfo(IRenderAuxGeom& auxGeom, Vec3 const& listenerPosition, float const posX, float posY) const;

#endif // INCLUDE_AUDIO_PRODUCTION_CODE

private:

	static float s_controlsUpdateInterval;

	bool HasActiveData(CATLAudioObject const* const pAudioObject) const;

	ConstructedAudioObjectsList  m_constructedAudioObjects;
	Impl::IImpl*                 m_pIImpl;
	CAudioEventUpr&          m_audioEventMgr;
	CAudioStandaloneFileUpr& m_audioStandaloneFileMgr;
	CAudioListenerUpr const& m_listenerUpr;
};
} //endns DrxAudio
