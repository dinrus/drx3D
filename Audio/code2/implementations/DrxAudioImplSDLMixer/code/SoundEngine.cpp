// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../SoundEngine.h"
#include "../SoundEngineUtil.h"
#include "../GlobalData.h"
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/Audio/Logger.h>
#include <drx3D/Sys/DrxFile.h>
#include <drx3D/CoreX/String/Path.h>

#include <SDL.h>
#include <SDL_mixer.h>

namespace DrxAudio
{
namespace Impl
{
namespace SDL_mixer
{
static string const s_projectPath = string(AUDIO_SYSTEM_DATA_ROOT) + "/" + s_szImplFolderName + "/" + s_szAssetsFolderName + "/";
static constexpr i32 s_supportedFormats = MIX_INIT_OGG | MIX_INIT_MP3;
static constexpr i32 s_numMixChannels = 512;
static constexpr SampleId s_invalidSampleId = 0;
static constexpr i32 s_sampleRate = 48000;
static constexpr i32 s_bufferSize = 4096;

// Samples
string g_sampleDataRootDir;
using SampleDataMap = std::unordered_map<SampleId, Mix_Chunk*>;
SampleDataMap g_sampleData;

using SampleNameMap = std::unordered_map<SampleId, string>;
SampleNameMap g_samplePaths;

using SampleIdUsageCounterMap = std::unordered_map<SampleId, i32>;
SampleIdUsageCounterMap g_usageCounters;

// Channels
struct SChannelData
{
	CObject* pObject;
};

SChannelData g_channels[s_numMixChannels];

using TChannelQueue = std::queue<i32>;
TChannelQueue g_freeChannels;

enum class EChannelFinishedRequestQueueId : EnumFlagsType
{
	One,
	Two,
	Count
};
DRX_CREATE_ENUM_FLAG_OPERATORS(EChannelFinishedRequestQueueId);

using ChannelFinishedRequests = std::deque<i32>;
ChannelFinishedRequests g_channelFinishedRequests[IntegralValue(EChannelFinishedRequestQueueId::Count)];
DrxCriticalSection g_channelFinishedCriticalSection;

// Audio Objects
using Objects = std::vector<CObject*>;
Objects g_objects;

// Listeners
CObjectTransformation g_listenerTransformation;
bool g_bListenerPosChanged;
bool g_bMuted;

SoundEngine::FnEventCallback g_fnEventFinishedCallback;
SoundEngine::FnStandaloneFileCallback g_fnStandaloneFileFinishedCallback;

//////////////////////////////////////////////////////////////////////////
void SoundEngine::RegisterEventFinishedCallback(FnEventCallback pCallbackFunction)
{
	g_fnEventFinishedCallback = pCallbackFunction;
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::RegisterStandaloneFileFinishedCallback(FnStandaloneFileCallback pCallbackFunction)
{
	g_fnStandaloneFileFinishedCallback = pCallbackFunction;
}

//////////////////////////////////////////////////////////////////////////
void EventFinishedPlaying(CATLEvent& audioEvent)
{
	if (g_fnEventFinishedCallback)
	{
		g_fnEventFinishedCallback(audioEvent);
	}
}

//////////////////////////////////////////////////////////////////////////
void StandaloneFileFinishedPlaying(CATLStandaloneFile& standaloneFile, char const* const szFileName)
{
	if (g_fnStandaloneFileFinishedCallback)
	{
		g_fnStandaloneFileFinishedCallback(standaloneFile, szFileName);
	}
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::UnloadSample(const SampleId nID)
{
	Mix_Chunk* pSample = stl::find_in_map(g_sampleData, nID, nullptr);
	if (pSample != nullptr)
	{
		Mix_FreeChunk(pSample);
		stl::member_find_and_erase(g_sampleData, nID);
	}
	else
	{
		Drx::Audio::Log(ELogType::Error, "Could not find sample with id %d", nID);
	}
}

//////////////////////////////////////////////////////////////////////////
void ProcessChannelFinishedRequests(ChannelFinishedRequests& queue)
{
	if (!queue.empty())
	{
		for (i32k finishedChannelId : queue)
		{
			CObject* pAudioObject = g_channels[finishedChannelId].pObject;
			if (pAudioObject)
			{
				EventInstanceList::iterator eventsEnd = pAudioObject->m_events.end();
				for (EventInstanceList::iterator eventsIt = pAudioObject->m_events.begin(); eventsIt != eventsEnd; )
				{
					CEvent* pEventInstance = *eventsIt;
					EventInstanceList::iterator eventsCurrent = eventsIt;
					++eventsIt;
					if (pEventInstance)
					{
						const ChannelList::iterator channelsEnd = pEventInstance->m_channels.end();
						for (ChannelList::iterator channelIt = pEventInstance->m_channels.begin(); channelIt != channelsEnd; ++channelIt)
						{
							if (*channelIt == finishedChannelId)
							{
								pEventInstance->m_channels.erase(channelIt);
								if (pEventInstance->m_channels.empty())
								{
									eventsIt = pAudioObject->m_events.erase(eventsCurrent);
									eventsEnd = pAudioObject->m_events.end();
									EventFinishedPlaying(pEventInstance->m_event);
								}
								break;
							}
						}
					}
				}
				StandAloneFileInstanceList::iterator standaloneFilesEnd = pAudioObject->m_standaloneFiles.end();
				for (StandAloneFileInstanceList::iterator standaloneFilesIt = pAudioObject->m_standaloneFiles.begin(); standaloneFilesIt != standaloneFilesEnd; )
				{
					CStandaloneFile* pStandaloneFileInstance = *standaloneFilesIt;
					StandAloneFileInstanceList::iterator standaloneFilesCurrent = standaloneFilesIt;
					++standaloneFilesIt;
					if (pStandaloneFileInstance)
					{
						const ChannelList::iterator channelsEnd = pStandaloneFileInstance->m_channels.end();
						for (ChannelList::iterator channelIt = pStandaloneFileInstance->m_channels.begin(); channelIt != channelsEnd; ++channelIt)
						{
							if (*channelIt == finishedChannelId)
							{
								pStandaloneFileInstance->m_channels.erase(channelIt);
								if (pStandaloneFileInstance->m_channels.empty())
								{
									standaloneFilesIt = pAudioObject->m_standaloneFiles.erase(standaloneFilesCurrent);
									standaloneFilesEnd = pAudioObject->m_standaloneFiles.end();
									StandaloneFileFinishedPlaying(pStandaloneFileInstance->m_atlFile, pStandaloneFileInstance->m_name.c_str());

									SampleIdUsageCounterMap::iterator it = g_usageCounters.find(pStandaloneFileInstance->m_sampleId);
									DRX_ASSERT(it != g_usageCounters.end() && it->second > 0);
									if (--it->second == 0)
									{
										SoundEngine::UnloadSample(pStandaloneFileInstance->m_sampleId);
									}
								}
								break;
							}
						}
					}
				}
				g_channels[finishedChannelId].pObject = nullptr;
			}
			g_freeChannels.push(finishedChannelId);
		}
		queue.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
void ChannelFinishedPlaying(i32 nChannel)
{
	if (nChannel >= 0 && nChannel < s_numMixChannels)
	{
		DrxAutoLock<DrxCriticalSection> autoLock(g_channelFinishedCriticalSection);
		g_channelFinishedRequests[IntegralValue(EChannelFinishedRequestQueueId::One)].push_back(nChannel);
	}
}

//////////////////////////////////////////////////////////////////////////
void LoadMetadata(const string& path)
{
	_finddata_t fd;
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	intptr_t handle = pDrxPak->FindFirst(g_sampleDataRootDir + path + "*.*", &fd);
	if (handle != -1)
	{
		do
		{
			const string name = fd.name;
			if (name != "." && name != ".." && !name.empty())
			{
				if (fd.attrib & _A_SUBDIR)
				{
					LoadMetadata(path + name + "/");
				}
				else
				{
					if (name.find(".wav") != string::npos ||
					    name.find(".ogg") != string::npos ||
					    name.find(".mp3") != string::npos)
					{
						if (path.empty())
						{
							g_samplePaths[GetIDFromString(name)] = g_sampleDataRootDir + name;
						}
						else
						{
							string pathName = path + name;
							g_samplePaths[GetIDFromString(pathName)] = g_sampleDataRootDir + pathName;
						}
					}
				}
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);
		pDrxPak->FindClose(handle);
	}
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::Init()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		Drx::Audio::Log(ELogType::Error, "SDL::SDL_Init() returned: %s", SDL_GetError());
		return false;
	}

	i32 loadedFormats = Mix_Init(s_supportedFormats);
	if ((loadedFormats & s_supportedFormats) != s_supportedFormats)
	{
		Drx::Audio::Log(ELogType::Error, R"(SDLMixer::Mix_Init() failed to init support for format flags %d with error "%s")", s_supportedFormats, Mix_GetError());
		return false;
	}

	if (Mix_OpenAudio(s_sampleRate, MIX_DEFAULT_FORMAT, 2, s_bufferSize) < 0)
	{
		Drx::Audio::Log(ELogType::Error, R"(SDLMixer::Mix_OpenAudio() failed to init the SDL Mixer API with error "%s")", Mix_GetError());
		return false;
	}

	Mix_AllocateChannels(s_numMixChannels);

	g_bMuted = false;
	Mix_Volume(-1, SDL_MIX_MAXVOLUME);
	for (i32 i = 0; i < s_numMixChannels; ++i)
	{
		g_freeChannels.push(i);
	}

	Mix_ChannelFinished(ChannelFinishedPlaying);

	g_sampleDataRootDir = PathUtil::GetPathWithoutFilename(s_projectPath);
	LoadMetadata("");
	g_bListenerPosChanged = false;

	// need to reinit as the global variable might have been initialized with wrong values
	g_listenerTransformation = CObjectTransformation();

	g_objects.reserve(128);

	return true;
}

//////////////////////////////////////////////////////////////////////////
void FreeAllSampleData()
{
	Mix_HaltChannel(-1);
	SampleDataMap::const_iterator it = g_sampleData.begin();
	SampleDataMap::const_iterator end = g_sampleData.end();
	for (; it != end; ++it)
	{
		Mix_FreeChunk(it->second);
	}
	g_sampleData.clear();
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Release()
{
	FreeAllSampleData();
	g_objects.clear();
	g_objects.shrink_to_fit();
	Mix_Quit();
	Mix_CloseAudio();
	SDL_Quit();
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Refresh()
{
	FreeAllSampleData();
	LoadMetadata(s_projectPath);
}

//////////////////////////////////////////////////////////////////////////
const SampleId SoundEngine::LoadSampleFromMemory(uk pMemory, const size_t size, const string& samplePath, const SampleId overrideId)
{
	const SampleId id = (overrideId != 0) ? overrideId : GetIDFromString(samplePath);
	Mix_Chunk* pSample = stl::find_in_map(g_sampleData, id, nullptr);
	if (pSample != nullptr)
	{
		Mix_FreeChunk(pSample);
		Drx::Audio::Log(ELogType::Warning, "Loading sample %s which had already been loaded", samplePath.c_str());
	}
	SDL_RWops* pData = SDL_RWFromMem(pMemory, size);
	if (pData)
	{
		pSample = Mix_LoadWAV_RW(pData, 0);
		if (pSample != nullptr)
		{
			g_sampleData[id] = pSample;
			g_samplePaths[id] = samplePath;
			return id;
		}
		else
		{
			Drx::Audio::Log(ELogType::Error, R"(SDL Mixer failed to load sample. Error: "%s")", Mix_GetError());
		}
	}
	else
	{
		Drx::Audio::Log(ELogType::Error, R"(SDL Mixer failed to transform the audio data. Error: "%s")", SDL_GetError());
	}
	return s_invalidSampleId;
}

//////////////////////////////////////////////////////////////////////////
bool LoadSampleImpl(const SampleId id, const string& samplePath)
{
	bool bSuccess = true;
	Mix_Chunk* pSample = Mix_LoadWAV(samplePath.c_str());
	if (pSample != nullptr)
	{
#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
		SampleNameMap::const_iterator it = g_samplePaths.find(id);
		if (it != g_samplePaths.end() && it->second != samplePath)
		{
			Drx::Audio::Log(ELogType::Error, "Loaded a Sample with the already existing ID %u, but from a different path source path '%s' <-> '%s'.", static_cast<uint>(id), it->second.c_str(), samplePath.c_str());
		}
		if (stl::find_in_map(g_sampleData, id, nullptr) != nullptr)
		{
			Drx::Audio::Log(ELogType::Error, "Loading sample '%s' which had already been loaded", samplePath.c_str());
		}
#endif
		g_sampleData[id] = pSample;
		g_samplePaths[id] = samplePath;
	}
	else
	{
		// Sample could be inside a pak file so we need to open it manually and load it from the raw file
		const size_t fileSize = gEnv->pDrxPak->FGetSize(samplePath);
		FILE* const pFile = gEnv->pDrxPak->FOpen(samplePath, "rbx", IDrxPak::FOPEN_HINT_DIRECT_OPERATION);
		if (pFile && fileSize > 0)
		{
			uk const pData = DrxModuleMalloc(fileSize);
			gEnv->pDrxPak->FReadRawAll(pData, fileSize, pFile);
			const SampleId newId = SoundEngine::LoadSampleFromMemory(pData, fileSize, samplePath, id);
			if (newId == s_invalidSampleId)
			{
				Drx::Audio::Log(ELogType::Error, R"(SDL Mixer failed to load sample %s. Error: "%s")", samplePath.c_str(), Mix_GetError());
				bSuccess = false;
			}
			DrxModuleFree(pData);
		}
	}
	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
const SampleId SoundEngine::LoadSample(const string& sampleFilePath, bool bOnlyMetadata)
{
	const SampleId id = GetIDFromString(sampleFilePath);
	if (stl::find_in_map(g_sampleData, id, nullptr) == nullptr)
	{
		if (bOnlyMetadata)
		{
			g_samplePaths[id] = g_sampleDataRootDir + sampleFilePath;
		}
		else if (!LoadSampleImpl(id, sampleFilePath))
		{
			return s_invalidSampleId;
		}
	}
	return id;
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Pause()
{
	Mix_Pause(-1);
	Mix_PauseMusic();
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Resume()
{
	Mix_Resume(-1);
	Mix_ResumeMusic();
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Stop()
{
	Mix_HaltChannel(-1);
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Mute()
{
	Mix_Volume(-1, 0);
	g_bMuted = true;
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::UnMute()
{
	Objects::const_iterator audioObjectIt = g_objects.begin();
	Objects::const_iterator const audioObjectEnd = g_objects.end();

	for (; audioObjectIt != audioObjectEnd; ++audioObjectIt)
	{
		CObject* const pAudioObject = *audioObjectIt;

		if (pAudioObject != nullptr)
		{
			EventInstanceList::const_iterator eventIt = pAudioObject->m_events.begin();
			EventInstanceList::const_iterator const eventEnd = pAudioObject->m_events.end();

			for (; eventIt != eventEnd; ++eventIt)
			{
				CEvent* const pEventInstance = *eventIt;

				if (pEventInstance != nullptr)
				{
					auto const pTrigger = pEventInstance->m_pTrigger;

					if (pTrigger != nullptr)
					{
						float const volumeMultiplier = GetVolumeMultiplier(pAudioObject, pTrigger->GetSampleId());
						i32 const mixVolume = GetAbsoluteVolume(pTrigger->GetVolume(), volumeMultiplier);

						ChannelList::const_iterator channelIt = pEventInstance->m_channels.begin();
						ChannelList::const_iterator const channelEnd = pEventInstance->m_channels.end();

						for (; channelIt != channelEnd; ++channelIt)
						{
							Mix_Volume(*channelIt, mixVolume);
						}
					}
				}
			}
		}
	}

	g_bMuted = false;
}

//////////////////////////////////////////////////////////////////////////
void SetChannelPosition(const CTrigger* pStaticData, i32k channelID, const float distance, const float angle)
{
	static u8k sdlMaxDistance = 255;
	const float min = pStaticData->GetAttenuationMinDistance();
	const float max = pStaticData->GetAttenuationMaxDistance();

	if (min <= max)
	{
		u8 nDistance = 0;

		if (max >= 0.0f && distance > min)
		{
			if (min != max)
			{
				const float finalDistance = distance - min;
				const float range = max - min;
				nDistance = static_cast<u8>((std::min((finalDistance / range), 1.0f) * sdlMaxDistance) + 0.5f);
			}
			else
			{
				nDistance = sdlMaxDistance;
			}
		}
		//Temp code, to be reviewed during the SetChannelPosition rewrite:
		Mix_SetDistance(channelID, nDistance);

		if (pStaticData->IsPanningEnabled())
		{
			//Temp code, to be reviewed during the SetChannelPosition rewrite:
			float const absAngle = fabs(angle);
			float const frontAngle = (angle > 0.0f ? 1.0f : -1.0f) * (absAngle > 90.0f ? 180.f - absAngle : absAngle);
			float const rightVolume = (frontAngle + 90.0f) / 180.0f;
			float const leftVolume = 1.0f - rightVolume;
			Mix_SetPanning(channelID,
			               static_cast<u8>(255.0f * leftVolume),
			               static_cast<u8>(255.0f * rightVolume));
		}
	}
	else
	{
		Drx::Audio::Log(ELogType::Error, "The minimum attenuation distance value is higher than the maximum");
	}
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::StopEvent(CEvent const* const pEvent)
{
	if (pEvent != nullptr)
	{
		// need to make a copy because the callback
		// registered with Mix_ChannelFinished can edit the list
		ChannelList const channels = pEvent->m_channels;
		i32 const fadeOutTime = pEvent->m_pTrigger->GetFadeOutTime();

		for (i32 const channel : channels)
		{
			if (fadeOutTime == 0)
			{
				Mix_HaltChannel(channel);
			}
			else
			{
				Mix_FadeOutChannel(channel, fadeOutTime);
			}
		}

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::PauseEvent(CEvent const* const pEvent)
{
	if (pEvent != nullptr)
	{
		// need to make a copy because the callback
		// registered with Mix_ChannelFinished can edit the list
		ChannelList const channels = pEvent->m_channels;

		for (i32 const channel : channels)
		{
			Mix_Pause(channel);
		}

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::ResumeEvent(CEvent const* const pEvent)
{
	if (pEvent != nullptr)
	{
		// need to make a copy because the callback
		// registered with Mix_ChannelFinished can edit the list
		ChannelList channels = pEvent->m_channels;

		for (i32 const channel : channels)
		{
			Mix_Resume(channel);
		}

		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
ERequestStatus SoundEngine::ExecuteEvent(CObject* const pObject, CTrigger const* const pTrigger, CEvent* const pEvent)
{
	ERequestStatus requestStatus = ERequestStatus::Failure;

	if ((pObject != nullptr) && (pTrigger != nullptr) && (pEvent != nullptr))
	{
		EEventType const type = pTrigger->GetType();
		SampleId const sampleId = pTrigger->GetSampleId();

		if (type == EEventType::Start)
		{
			// Start playing samples
			pEvent->m_pTrigger = pTrigger;
			Mix_Chunk* pSample = stl::find_in_map(g_sampleData, sampleId, nullptr);

			if (pSample == nullptr)
			{
				// Trying to play sample that hasn't been loaded yet, load it in place
				// NOTE: This should be avoided as it can cause lag in audio playback
				string const& samplePath = g_samplePaths[sampleId];

				if (LoadSampleImpl(sampleId, samplePath))
				{
					pSample = stl::find_in_map(g_sampleData, sampleId, nullptr);
				}

				if (pSample == nullptr)
				{
					return ERequestStatus::Failure;
				}
			}

			if (!g_freeChannels.empty())
			{
				i32 const channelID = g_freeChannels.front();

				if (channelID >= 0)
				{
					g_freeChannels.pop();

					float const volumeMultiplier = GetVolumeMultiplier(pObject, sampleId);
					i32 const mixVolume = GetAbsoluteVolume(pTrigger->GetVolume(), volumeMultiplier);
					Mix_Volume(channelID, g_bMuted ? 0 : mixVolume);

					i32 const fadeInTime = pTrigger->GetFadeInTime();
					i32 const loopCount = pTrigger->GetNumLoops();

					if (fadeInTime > 0)
					{
						Mix_FadeInChannel(channelID, pSample, loopCount, fadeInTime);
					}
					else
					{
						Mix_PlayChannel(channelID, pSample, loopCount);
					}

					// Get distance and angle from the listener to the audio object
					float distance = 0.0f;
					float angle = 0.0f;
					GetDistanceAngleToObject(g_listenerTransformation, pObject->m_transformation, distance, angle);
					SetChannelPosition(pEvent->m_pTrigger, channelID, distance, angle);

					g_channels[channelID].pObject = pObject;
					pEvent->m_channels.push_back(channelID);
				}
				else
				{
					Drx::Audio::Log(ELogType::Error, "Could not play sample. Error: %s", Mix_GetError());
				}
			}
			else
			{
				Drx::Audio::Log(ELogType::Error, "Ran out of free audio channels. Are you trying to play more than %d samples?", s_numMixChannels);
			}

			if (!pEvent->m_channels.empty())
			{
				// If any sample was added then add the event to the audio object
				pObject->m_events.push_back(pEvent);
				requestStatus = ERequestStatus::Success;
			}
		}
		else
		{
			for (auto const pEventToProcess : pObject->m_events)
			{
				if (pEventToProcess->m_pTrigger->GetSampleId() == sampleId)
				{
					switch (type)
					{
					case EEventType::Stop:
						SoundEngine::StopEvent(pEventToProcess);
						break;
					case EEventType::Pause:
						SoundEngine::PauseEvent(pEventToProcess);
						break;
					case EEventType::Resume:
						SoundEngine::ResumeEvent(pEventToProcess);
						break;
					}
				}
			}

			requestStatus = ERequestStatus::SuccessDoNotTrack;
		}
	}

	return requestStatus;
}

//////////////////////////////////////////////////////////////////////////
ERequestStatus SoundEngine::SetVolume(CObject* const pObject, SampleId const sampleId)
{
	if (!g_bMuted)
	{
		float const volumeMultiplier = GetVolumeMultiplier(pObject, sampleId);

		for (auto const pEvent : pObject->m_events)
		{
			auto const pTrigger = pEvent->m_pTrigger;

			if ((pTrigger != nullptr) && (pTrigger->GetSampleId() == sampleId))
			{
				i32 const mixVolume = GetAbsoluteVolume(pTrigger->GetVolume(), volumeMultiplier);

				for (auto const channel : pEvent->m_channels)
				{
					Mix_Volume(channel, mixVolume);
				}
			}
		}
	}

	return ERequestStatus::Success;
}

//////////////////////////////////////////////////////////////////////////
float SoundEngine::GetVolumeMultiplier(CObject* const pObject, SampleId const sampleId)
{
	float volumeMultiplier = 1.0f;
	auto const volumeMultiplierPair = pObject->m_volumeMultipliers.find(sampleId);

	if (volumeMultiplierPair != pObject->m_volumeMultipliers.end())
	{
		volumeMultiplier = volumeMultiplierPair->second;
	}

	return volumeMultiplier;
}

//////////////////////////////////////////////////////////////////////////
i32 SoundEngine::GetAbsoluteVolume(i32 const triggerVolume, float const multiplier)
{
	i32 absoluteVolume = static_cast<i32>(static_cast<float>(triggerVolume) * multiplier);
	absoluteVolume = drxmath::clamp(absoluteVolume, 0, 128);
	return absoluteVolume;
}
//////////////////////////////////////////////////////////////////////////
bool SoundEngine::PlayFile(CObject* const pObject, CStandaloneFile* const pStandaloneFile)
{
	SampleId idForThisFile = pStandaloneFile->m_sampleId;
	Mix_Chunk* pSample = stl::find_in_map(g_sampleData, idForThisFile, nullptr);

	if (!pSample)
	{
		if (LoadSampleImpl(idForThisFile, pStandaloneFile->m_name.c_str()))
		{
			pSample = stl::find_in_map(g_sampleData, idForThisFile, nullptr);
		}
		if (!pSample)
		{
			return false;
		}
	}

	SampleIdUsageCounterMap::iterator it = g_usageCounters.find(idForThisFile);
	if (it != g_usageCounters.end())
	{
		++it->second;
	}
	else
	{
		g_usageCounters[idForThisFile] = 1;
	}

	if (!g_freeChannels.empty())
	{
		i32 channelId = Mix_PlayChannel(g_freeChannels.front(), pSample, 1);
		if (channelId >= 0)
		{
			g_freeChannels.pop();
			Mix_Volume(channelId, g_bMuted ? 0 : 128);

			// Get distance and angle from the listener to the audio object
			float distance = 0.0f;
			float angle = 0.0f;
			GetDistanceAngleToObject(g_listenerTransformation, pObject->m_transformation, distance, angle);

			// Assuming a max distance of 100.0
			u8 sldMixerDistance = static_cast<u8>((std::min((distance / 100.0f), 1.0f) * 255) + 0.5f);

			Mix_SetDistance(channelId, sldMixerDistance);

			float const absAngle = fabs(angle);
			float const frontAngle = (angle > 0.0f ? 1.0f : -1.0f) * (absAngle > 90.0f ? 180.f - absAngle : absAngle);
			float const rightVolume = (frontAngle + 90.0f) / 180.0f;
			float const leftVolume = 1.0f - rightVolume;
			Mix_SetPanning(channelId, static_cast<u8>(255.0f * leftVolume), static_cast<u8>(255.0f * rightVolume));

			g_channels[channelId].pObject = pObject;
			pStandaloneFile->m_channels.push_back(channelId);
		}
		else
		{
			Drx::Audio::Log(ELogType::Error, "Could not play sample. Error: %s", Mix_GetError());
		}
	}
	else
	{
		Drx::Audio::Log(ELogType::Error, "Ran out of free audio channels. Are you trying to play more than %d samples?", s_numMixChannels);
	}

	if (!pStandaloneFile->m_channels.empty())
	{
		// if any sample was added then add the event to the audio object
		pObject->m_standaloneFiles.push_back(pStandaloneFile);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::StopFile(CObject* const pObject, CStandaloneFile* const pStandaloneFile)
{
	bool bResult = false;

	if (pObject != nullptr)
	{
		for (CStandaloneFile* const pTempStandaloneFile : pObject->m_standaloneFiles)
		{
			if (pTempStandaloneFile == pStandaloneFile)
			{
				// need to make a copy because the callback
				// registered with Mix_ChannelFinished can edit the list
				ChannelList channels = pStandaloneFile->m_channels;
				for (i32 channel : channels)
				{
					Mix_HaltChannel(channel);
				}
				bResult = true;
			}
		}
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::SetListenerPosition(ListenerId const listenerId, CObjectTransformation const& transformation)
{
	g_listenerTransformation = transformation;
	g_bListenerPosChanged = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::RegisterObject(CObject* const pObject)
{
	if (pObject != nullptr)
	{
		g_objects.push_back(pObject);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::UnregisterObject(CObject const* const pObject)
{
	if (pObject != nullptr)
	{
		stl::find_and_erase(g_objects, pObject);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::SetObjectTransformation(CObject* const pObject, CObjectTransformation const& transformation)
{
	if (pObject != nullptr)
	{
		pObject->m_transformation = transformation;
		pObject->m_bPositionChanged = true;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool SoundEngine::StopTrigger(CTrigger const* const pTrigger)
{
	bool bResult = false;

	for (auto const pObject : g_objects)
	{
		if (pObject != nullptr)
		{
			for (auto const pEvent : pObject->m_events)
			{
				if (pEvent != nullptr && pEvent->m_pTrigger == pTrigger)
				{
					SoundEngine::StopEvent(pEvent);
					bResult = true;
				}
			}
		}
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
void SoundEngine::Update()
{
	{
		DrxAutoLock<DrxCriticalSection> lock(g_channelFinishedCriticalSection);
		g_channelFinishedRequests[IntegralValue(EChannelFinishedRequestQueueId::One)].swap(g_channelFinishedRequests[IntegralValue(EChannelFinishedRequestQueueId::Two)]);
	}

	ProcessChannelFinishedRequests(g_channelFinishedRequests[IntegralValue(EChannelFinishedRequestQueueId::Two)]);

	for (auto const pObject : g_objects)
	{
		if (pObject != nullptr)
		{
			// Get distance and angle from the listener to the audio object
			float distance = 0.0f;
			float angle = 0.0f;
			GetDistanceAngleToObject(g_listenerTransformation, pObject->m_transformation, distance, angle);

			for (auto const pEvent : pObject->m_events)
			{
				if (pEvent != nullptr && pEvent->m_pTrigger != nullptr)
				{
					for (auto const channelIndex : pEvent->m_channels)
					{
						SetChannelPosition(pEvent->m_pTrigger, channelIndex, distance, angle);
					}
				}

				pObject->m_bPositionChanged = false;
			}
		}
	}

	g_bListenerPosChanged = false;
}
} //endns SDL_mixer
} //endns Impl
} //endns DrxAudio
