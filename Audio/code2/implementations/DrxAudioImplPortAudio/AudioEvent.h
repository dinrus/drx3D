// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/ATLEntityData.h>
#include <drx/plugin/portaudio/portaudio.h>
#include <atomic>
#include <drx3D/Audio/PoolObject.h>

// Forward declare C struct
struct sf_private_tag;
using SNDFILE = struct sf_private_tag;

namespace DrxAudio
{
namespace Impl
{
namespace PortAudio
{
class CObject;

class CEvent final : public IEvent, public CPoolObject<CEvent, stl::PSyncNone>
{
public:

	explicit CEvent(CATLEvent& event_);
	virtual ~CEvent() override;

	CEvent(CEvent const&) = delete;
	CEvent(CEvent&&) = delete;
	CEvent& operator=(CEvent const&) = delete;
	CEvent& operator=(CEvent&&) = delete;

	bool    Execute(
	  i32 const numLoops,
	  double const sampleRate,
	  DrxFixedStringT<MaxFilePathLength> const& filePath,
	  PaStreamParameters const& streamParameters);
	void Update();

	// DrxAudio::Impl::IEvent
	virtual ERequestStatus Stop() override;
	// ~DrxAudio::Impl::IEvent

	SNDFILE*          pSndFile;
	PaStream*         pStream;
	uk             pData;
	CObject*          pObject;
	i32               numChannels;
	i32               remainingLoops;
	CATLEvent&        event;
	u32            pathId;
	PaSampleFormat    sampleFormat;
	std::atomic<bool> bDone;
};
} //endns PortAudio
} //endns Impl
} //endns DrxAudio
