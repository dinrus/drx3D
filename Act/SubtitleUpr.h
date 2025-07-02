// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ISubtitleUpr.h"

class CSubtitleUpr : public ISubtitleUpr
{
public:
	CSubtitleUpr();
	virtual ~CSubtitleUpr();

	// ISubtitleUpr
	virtual void SetHandler(ISubtitleHandler* pHandler) { m_pHandler = pHandler; }
	virtual void SetEnabled(bool bEnabled);
	virtual void SetAutoMode(bool bOn);
	virtual void ShowSubtitle(tukk subtitleLabel, bool bShow);
	// ~ISubtitleUpr

	static void OnAudioTriggerStarted(const DrxAudio::SRequestInfo* const pAudioRequestInfo);
	static void OnAudioTriggerFinished(const DrxAudio::SRequestInfo* const pAudioRequestInfo);

protected:
	void ShowSubtitle(const DrxAudio::SRequestInfo* const pAudioRequestInfo, bool bShow);

	ISubtitleHandler*        m_pHandler;
	bool                     m_bEnabled;
	bool                     m_bAutoMode;

	static CSubtitleUpr* s_Instance;
};
