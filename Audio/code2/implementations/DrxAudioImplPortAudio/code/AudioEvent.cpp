// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "../stdafx.h"
#include "../AudioEvent.h"
#include "../AudioObject.h"
#include <drx3D/Audio/Logger.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/Sys/ISystem.h> // needed for gEnv in Release builds
#include <drx3D/Plugins/portaudio/portaudio.h>
#include <sndfile.hh>

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
{
static long unsigned const s_bufferLength = 256;

// Callbacks
//////////////////////////////////////////////////////////////////////////
static i32 StreamCallback(
  void const* pInputBuffer,
  uk pOutputBuffer,
  long unsigned framesPerBuffer,
  PaStreamCallbackTimeInfo const* pTimeInfo,
  PaStreamCallbackFlags statusFlags,
  uk pUserData)
{
	DRX_ASSERT(framesPerBuffer == s_bufferLength);
	sf_count_t numFramesRead = 0;
	CEvent* const pAudioEvent = static_cast<CEvent*>(pUserData);

	switch (pAudioEvent->sampleFormat)
	{
	case paInt16:
		{
			short* pStreamData = static_cast<short*>(pOutputBuffer);
			numFramesRead = sf_readf_short(pAudioEvent->pSndFile, static_cast<short*>(pAudioEvent->pData), static_cast<sf_count_t>(s_bufferLength));

			for (sf_count_t i = 0; i < numFramesRead; ++i)
			{
				for (i32 j = 0; j < pAudioEvent->numChannels; ++j)
				{
					*pStreamData++ = static_cast<short*>(pAudioEvent->pData)[i * pAudioEvent->numChannels + j];
				}
			}
		}
		break;
	case paInt32:
		{
			i32* pStreamData = static_cast<i32*>(pOutputBuffer);
			numFramesRead = sf_readf_int(pAudioEvent->pSndFile, static_cast<i32*>(pAudioEvent->pData), static_cast<sf_count_t>(s_bufferLength));

			for (sf_count_t i = 0; i < numFramesRead; ++i)
			{
				for (i32 j = 0; j < pAudioEvent->numChannels; ++j)
				{
					*pStreamData++ = static_cast<i32*>(pAudioEvent->pData)[i * pAudioEvent->numChannels + j];
				}
			}
		}
		break;
	case paFloat32:
		{
			float* pStreamData = static_cast<float*>(pOutputBuffer);
			numFramesRead = sf_readf_float(pAudioEvent->pSndFile, static_cast<float*>(pAudioEvent->pData), static_cast<sf_count_t>(s_bufferLength));

			for (sf_count_t i = 0; i < numFramesRead; ++i)
			{
				for (i32 j = 0; j < pAudioEvent->numChannels; ++j)
				{
					*pStreamData++ = static_cast<float*>(pAudioEvent->pData)[i * pAudioEvent->numChannels + j];
				}
			}
		}
		break;
	}

	if (numFramesRead != framesPerBuffer)
	{
		pAudioEvent->bDone = true;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
CEvent::CEvent(CATLEvent& event_)
	: pSndFile(nullptr)
	, pStream(nullptr)
	, pData(nullptr)
	, pObject(nullptr)
	, numChannels(0)
	, remainingLoops(0)
	, event(event_)
	, bDone(false)
{
}

//////////////////////////////////////////////////////////////////////////
CEvent::~CEvent()
{
	if (pStream != nullptr)
	{
		PaError const err = Pa_CloseStream(pStream);

		if (err != paNoError)
		{
			Drx::Audio::Log(ELogType::Error, "CloseStream failed: %s", Pa_GetErrorText(err));
		}
	}

	if (pSndFile != nullptr)
	{
		sf_close(pSndFile);
	}

	if (pData != nullptr)
	{
		delete pData;
	}

	if (pObject != nullptr)
	{
		pObject->UnregisterEvent(this);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEvent::Execute(
  i32 const numLoops,
  double const sampleRate,
  DrxFixedStringT<MaxFilePathLength> const& filePath,
  PaStreamParameters const& streamParameters)
{
	bool bSuccess = false;
	SF_INFO sfInfo;
	ZeroStruct(sfInfo);
	pSndFile = sf_open(filePath.c_str(), SFM_READ, &sfInfo);

	if (pSndFile != nullptr)
	{
		PaError err = Pa_OpenStream(
		  &pStream,
		  nullptr,
		  &streamParameters,
		  sampleRate,
		  s_bufferLength,
		  paClipOff,
		  &StreamCallback,
		  static_cast<uk>(this));

		if (err == paNoError)
		{
			numChannels = streamParameters.channelCount;
			remainingLoops = numLoops;
			sampleFormat = streamParameters.sampleFormat;
			long unsigned const numEntries = s_bufferLength * numChannels;

			switch (sampleFormat)
			{
			case paInt16:
				{
					pData = DrxModuleMalloc(sizeof(short) * static_cast<size_t>(numEntries));
					memset(pData, 0, sizeof(short) * static_cast<size_t>(numEntries));
				}
				break;
			case paInt32:
				{
					pData = DrxModuleMalloc(sizeof(i32) * static_cast<size_t>(numEntries));
					memset(pData, 0, sizeof(i32) * static_cast<size_t>(numEntries));
				}
				break;
			case paFloat32:
				{
					pData = DrxModuleMalloc(sizeof(float) * static_cast<size_t>(numEntries));
					memset(pData, 0, sizeof(float) * static_cast<size_t>(numEntries));
				}
				break;
			}

			err = Pa_StartStream(pStream);

			if (err == paNoError)
			{
				bSuccess = true;
			}
			else
			{
				Drx::Audio::Log(ELogType::Error, "StartStream failed: %s", Pa_GetErrorText(err));
			}
		}
		else
		{
			Drx::Audio::Log(ELogType::Error, "OpenStream failed: %s", Pa_GetErrorText(err));
		}
	}

	return bSuccess;
}

//////////////////////////////////////////////////////////////////////////
void CEvent::Update()
{
	if (bDone)
	{
		if (remainingLoops != 0 && pStream != nullptr && pSndFile != nullptr)
		{
			sf_seek(pSndFile, 0, SEEK_SET);

			if (remainingLoops > 0)
			{
				--remainingLoops;
			}
		}
		else
		{
			gEnv->pAudioSystem->ReportFinishedEvent(event, true);
		}

		bDone = false;
	}
}

//////////////////////////////////////////////////////////////////////////
ERequestStatus CEvent::Stop()
{
	gEnv->pAudioSystem->ReportFinishedEvent(event, true);
	return ERequestStatus::Success;
}
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
