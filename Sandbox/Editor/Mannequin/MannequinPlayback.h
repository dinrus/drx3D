// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MANN_PLAYBACK_PAGE__H__
#define __MANN_PLAYBACK_PAGE__H__

#include "MannequinBase.h"

class CFragmentPlayback : public CBasicAction
{
public:
	DEFINE_ACTION("FragmentPlayback");

	CFragmentPlayback(ActionScopes scopeMask, u32 flags);

	virtual void             Enter();
	virtual void             Exit();
	void                     TriggerExit();
	void                     SetFragment(FragmentID fragmentID, TagState fragTags, u32 option, float maxTime);
	void                     Restart(float time = -1.0f);
	void                     SetTime(float time, bool bForce = false);
	float                    GetTimeSinceInstall() const;
	float                    GetTimeMax() const { return m_maxTime; }
	bool                     ReachedEnd() const;
	virtual IAction::EStatus Update(float timePassed);
	virtual void             OnSequenceFinished(i32 layer, u32 scopeID);

	virtual i32              GetPriority() const { return 102; }

private:
	void InternalSetFragment(FragmentID fragmentID, TagState fragTags, u32 option);

	float m_timeSinceInstall;
	float m_maxTime;
};

//////////////////////////////////////////////////////////////////////////

class CFragmentSequencePlayback
{
public:
	CFragmentSequencePlayback(const CFragmentHistory& history, IActionController& actionController, EMannequinEditorMode mode, const ActionScopes& scopeMask = ACTION_SCOPES_ALL)
		:
		m_time(0.0f),
		m_maxTime(5.0f),
		m_curIdx(0),
		m_playScale(1.0f),
		m_history(history),
		m_actionController(actionController),
		m_mode(mode),
		m_scopeMask(scopeMask)
	{
	}
	~CFragmentSequencePlayback();

	void Restart(float time = -1.0f);
	void SetTime(float time, bool bForce = false);
	void Update(float time, class CMannequinModelViewport * pViewport);

	float GetTime() const
	{
		return m_time;
	}

	void SetScopeMask(const ActionScopes& scopeMask)
	{
		m_scopeMask = scopeMask;
	}

	void SetSpeedBias(float playScale);

private:

	void StopPrevious(ActionScopes scopeMask);
	void UpdateDebugParams();

	const CFragmentHistory& m_history;
	IActionController&      m_actionController;

	std::vector<IAction*>   m_actions;

	EMannequinEditorMode    m_mode;

	ActionScopes            m_scopeMask;

	float                   m_time;
	float                   m_maxTime;
	u32                  m_curIdx;
	float                   m_playScale;

};

#endif

