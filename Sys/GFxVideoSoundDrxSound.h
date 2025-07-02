// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _GFXVIDEOSOUNDDRXSOUND_H_
#define _GFXVIDEOSOUNDDRXSOUND_H_

#pragma once

#ifdef INCLUDE_SCALEFORM_SDK

namespace DrxVideoSoundSystem
{
	struct IChannelDelegate
	{
		// <interfuscator:shuffle>
		virtual void Release() = 0;

		virtual bool Stop()                = 0;
		virtual bool SetPaused(bool pause) = 0;

		virtual bool SetVolume(float volume) = 0;
		virtual bool Mute(bool mute)         = 0;

		virtual bool SetBytePosition(u32 bytePos)  = 0;
		virtual bool GetBytePosition(u32& bytePos) = 0;

		virtual bool SetSpeakerMix(float fl, float fr, float c, float lfe, float bl, float br, float sl, float sr) = 0;
		// </interfuscator:shuffle>

	protected:
		virtual ~IChannelDelegate() {}
	};

	struct ISoundDelegate
	{
		struct LockRange
		{
			uk p0;
			uk p1;
			u32 length0;
			u32 length1;
		};

		// <interfuscator:shuffle>
		virtual void Release() = 0;

		virtual IChannelDelegate* Play() = 0;

		virtual bool Lock(u32 offset, u32 length, LockRange& lr) = 0;
		virtual bool Unlock(const LockRange& lr) = 0;
		// </interfuscator:shuffle>

	protected:
		virtual ~ISoundDelegate() {}
	};

	struct IPlayerDelegate
	{
		// <interfuscator:shuffle>
		virtual void Release() = 0;

		virtual ISoundDelegate* CreateSound(u32 numChannels, u32 sampleRate, u32 lengthInBytes) = 0;

		virtual bool MuteMainTrack() const = 0;
		// </interfuscator:shuffle>

	protected:
		virtual ~IPlayerDelegate() {}
	};

	struct IAllocatorDelegate
	{
		// <interfuscator:shuffle>
		virtual uk Allocate(size_t size) = 0;
		virtual void  Free(uk p)         = 0;
		// </interfuscator:shuffle>

	protected:
		virtual ~IAllocatorDelegate() {}
	};
}                                        // namespace DrxVideoSoundSystem

#endif                                   //#ifdef INCLUDE_SCALEFORM_SDK

#ifdef INCLUDE_SCALEFORM_SDK

#include <drx3D/Sys/ConfigScaleform.h>

#if defined(USE_GFX_VIDEO)

#pragma warning(push)
#pragma warning(disable : 6326)          // Potential comparison of a constant with another constant
#pragma warning(disable : 6011)          // Dereferencing NULL pointer
#include <drx3D/CoreX/Platform/DrxWindows.h>
#include <GFxSystemSoundInterface.h>     // includes <windows.h>
#pragma warning(pop)

class GFxVideoDrxSoundSystemImpl;

class GFxVideoDrxSoundSystem:public GFxVideoSoundSystem
{
public:
	virtual GFxVideoSound* Create(GFxVideoPlayer::SoundTrack type);

public:
	GFxVideoDrxSoundSystem(GMemoryHeap* pHeap);
	virtual ~GFxVideoDrxSoundSystem();

public:
	static void InitCVars();

private:
	GFxVideoDrxSoundSystemImpl* m_pImpl;
};

#else

class GFxVideoDrxSoundSystem
{
public:
	static void InitCVars();

};

#endif   // #if defined(USE_GFX_VIDEO)

#endif   // #ifdef INCLUDE_SCALEFORM_SDK

#endif   // #ifndef _GFXVIDEOSOUNDDRXSOUND_H_
