// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
struct ISubtitleHandler
{
	virtual ~ISubtitleHandler(){}
	virtual void ShowSubtitle(const DrxAudio::SRequestInfo* const pAudioRequestInfo, bool bShow) = 0;
	virtual void ShowSubtitle(tukk subtitleLabel, bool bShow) = 0;
};

//////////////////////////////////////////////////////////////////////////
struct ISubtitleUpr
{
	virtual ~ISubtitleUpr(){}
	virtual void SetHandler(ISubtitleHandler* pHandler) = 0;

	// enables/disables subtitles manager
	virtual void SetEnabled(bool bEnabled) = 0;

	// automatic mode. Will inform the subtitleHandler about every executed/stopped audio trigger.
	// You can use this mode, if you want to drive your subtitles by started sounds and not manually.
	virtual void SetAutoMode(bool bOn) = 0;

	virtual void ShowSubtitle(tukk subtitleLabel, bool bShow) = 0;
};
