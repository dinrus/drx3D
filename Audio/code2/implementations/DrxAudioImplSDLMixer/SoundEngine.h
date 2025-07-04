// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SoundEngineTypes.h"

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
namespace SoundEngine
{
using FnEventCallback = void (*)(CATLEvent&);
using FnStandaloneFileCallback = void (*)(CATLStandaloneFile&, char const*);

// Global events
bool Init();
void Release();
void Refresh();
void Update();

void Pause();
void Resume();

void Mute();
void UnMute();
void Stop();

// Load / Unload samples
const SampleId LoadSample(const string& sampleFilePath, bool bOnlyMetadata);
const SampleId LoadSampleFromMemory(uk pMemory, const size_t size, const string& samplePath, const SampleId id = 0);
void           UnloadSample(const SampleId id);

// Events
ERequestStatus ExecuteEvent(CObject* const pObject, CTrigger const* const pTrigger, CEvent* const pEvent);
ERequestStatus SetVolume(CObject* const pObject, SampleId const sampleId);
float          GetVolumeMultiplier(CObject* const pObject, SampleId const sampleId);
i32            GetAbsoluteVolume(i32 const triggerVolume, float const multiplier);
bool           PlayFile(CObject* const pObject, CStandaloneFile* const pStandaloneFile);
bool           StopFile(CObject* const pObject, CStandaloneFile* const pStandaloneFile);

// stops an specific event instance
bool StopEvent(CEvent const* const pEvent);
// stops all the events associated with this trigger
bool StopTrigger(CTrigger const* const pTrigger);

bool PauseEvent(CEvent const* const pEvent);
bool ResumeEvent(CEvent const* const pEvent);

// Listeners
bool SetListenerPosition(ListenerId const listenerId, CObjectTransformation const& transformation);

// Audio Objects
bool RegisterObject(CObject* const pObject);
bool UnregisterObject(CObject const* const pObject);
bool SetObjectTransformation(CObject* const pObject, CObjectTransformation const& transformation);

// Callbacks
void RegisterEventFinishedCallback(FnEventCallback pCallbackFunction);
void RegisterStandaloneFileFinishedCallback(FnStandaloneFileCallback pCallbackFunction);
} //endns SoundEngine
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
